/**
 *
 * cb7 Comic Book Thumbnailer for KDE 4 v0.1
 * Creates cover page previews for cb7 comic-book files.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// cb7creator.cpp

#include "cb7creator.h"

#include <kdemacros.h>
#include <kmimetype.h>
#include <kstandarddirs.h>

#include <kdebug.h>
#include <ktempdir.h>
#include <kprocess.h>

#include <memory>

#include <QtCore/QFile>

// For KIO-Thumbnail debug outputs
#define KIO_THUMB 11371

extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new Cb7Creator;
    }
};

Cb7Creator::Cb7Creator() : m_comicCover( 0 ) {}

Cb7Creator::~Cb7Creator()
{
    m_stdOut.clear();
    m_stdErr.clear();
}

bool Cb7Creator::create( const QString &path, int width, int height, QImage &img )
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    bool result( false );

    // Detect mime type.
    const KMimeType::Ptr mime = KMimeType::findByFileContent( path );

    if ( mime->is( "application/x-cb7" ) || mime->name() == "application/x-7z-compressed" ) {
        // 7Z archive.
        result = Cb7Creator::extract7zImage( path );
    } else {
        result = false;
    }

    if( !m_comicCover || !result ) {
        kDebug( KIO_THUMB )<<"Error creating the cb7 thumbnail.";
        return false;
    }

    // Copy the extracted image over to KIO::ThumbCreator's img reference.
    img = m_comicCover->copy();
    delete m_comicCover;

    return result;
}

void Cb7Creator::filterImages( QStringList &entries, const bool sensitive=true )
{
    /// Sort based on condition, then remove non-image entries.
    if( sensitive ) {
        entries.sort();
    } else {
        QMap<QString, QString> entryMap;
        Q_FOREACH( const QString &entry, entries ) {
            entryMap.insert( entry.toLower(), entry );
        }
        entries = entryMap.values();
    }

    Q_FOREACH( const QString &entry, entries ) {
        if ( !( entry.endsWith( QLatin1String( ".gif" ), Qt::CaseInsensitive ) ||\
                entry.endsWith( QLatin1String( ".jpg" ), Qt::CaseInsensitive ) ||\
                entry.endsWith( QLatin1String( ".jpeg" ), Qt::CaseInsensitive ) ||\
                entry.endsWith( QLatin1String( ".png" ), Qt::CaseInsensitive ) ) ) {
                    entries.removeOne( entry );
        }
    }
}

bool Cb7Creator::extract7zImage( QString path )
{
    /// Extracts the cover image out of the .cb7 file.
    QString uzPath;
    bool available = is7zAvailable(uzPath);
    // Check if 7z is available. Get its path in 'uzPath'.
    if( !available ) {
        kDebug( KIO_THUMB )<<"A suitable version of 7z is not available. Exiting.";
        return false;
    } 
    QStringList entries;

    // Get the files and filter the images out.
    Cb7Creator::get7zFileList( entries, path, uzPath );
    Cb7Creator::filterImages( entries, true );

    // Clear previously used data arrays.
    m_stdOut.clear();
    m_stdErr.clear();

    // Extract the cover file alone. Use verbose paths.
    // 
    //
    KTempDir *cUn7zTempDir = new KTempDir();
    Cb7Creator::startProcess( uzPath, QStringList() << "x" << path << entries[0] << "-o" + cUn7zTempDir->name() );
    // Open cover file from the temp directory.
    QFile coverFile( cUn7zTempDir->name() + entries[0] );
    if ( !coverFile.open( QIODevice::ReadOnly ) ) {
        return false;
    }

    // Load cover file data into image.
    m_comicCover = new QImage();
    m_comicCover->loadFromData( coverFile.readAll() );
    coverFile.close();
    if ( !m_comicCover ) {
        return false;
    }

    // Temp directory's served its purpose.
    cUn7zTempDir->unlink();
    delete cUn7zTempDir;

    return true;
}


void Cb7Creator::get7zFileList( QStringList &entries, const QString path, const QString uzPath )
{
    Cb7Creator::startProcess( uzPath, QStringList() << "l" << path << "-slt" );  
    QStringList entries_mem = QString::fromLocal8Bit( m_stdOut ).split( '\n', QString::SkipEmptyParts );
    for(QStringList::iterator i= entries_mem.begin();i!= entries_mem.end(); ++i)
      if(i->startsWith("Path = ",Qt::CaseSensitive)){
	QString mem=*i;
	mem.remove(0,7);
	entries.append(mem);
      }
    entries.removeFirst(); //Remove archive file from entries
}

bool Cb7Creator::is7zAvailable( QString &uzPath )
{
    /// Check the standard paths to see if a suitable p7zip is available.
      uzPath = KStandardDirs::findExe( "7z" );
      if( uzPath.isEmpty() ) {
	uzPath = KStandardDirs::findExe( "7za" );
	if( uzPath.isEmpty() ) {
	  uzPath = KStandardDirs::findExe( "7zr" );
	}
      }
      if( !uzPath.isEmpty() ) {
      QProcess proc;
      proc.start( uzPath, QStringList() << "-h" );
      proc.waitForFinished( -1 );
      const QStringList lines = QString::fromLocal8Bit( proc.readAllStandardOutput() ).split\
      ( '\n', QString::SkipEmptyParts );
      if ( lines.first().contains( "7-Zip" ) ) {
	return true;
      }
    }
      return false;
}

void Cb7Creator::readProcessOut()
{
    /// Read all std::out data and store to the data array.
    if ( !m_process )
        return;

    m_stdOut += m_process->readAllStandardOutput();
}

void Cb7Creator::readProcessErr()
{
    /// Read available std:err data and kill process if there is any.
    if ( !m_process )
        return;

    m_stdErr += m_process->readAllStandardError();
    if ( !m_stdErr.isEmpty() )
    {
        m_process->kill();
        return;
    }
}

void Cb7Creator::finishedProcess( int exitCode, QProcess::ExitStatus exitStatus )
{
    /// Run when process finishes.
    Q_UNUSED( exitCode )
    if ( m_loop )
    {
        m_loop->exit( exitStatus == QProcess::CrashExit ? 1 : 0 );
    }
}

int Cb7Creator::startProcess( const QString processPath, const QStringList args )
{
    /// Run a process and store std::out, std::err data in their respective buffers.
    int ret = 0;

#if defined( Q_OS_WIN )
    m_process = new QProcess( this );
#else
    m_process = new KPtyProcess( this );
    m_process->setOutputChannelMode( KProcess::SeparateChannels );
#endif

    connect( m_process, SIGNAL( readyReadStandardOutput() ), SLOT( readProcessOut() ) );
    connect( m_process, SIGNAL( readyReadStandardError() ), SLOT( readProcessErr() ) );
    connect( m_process, SIGNAL( finished( int, QProcess::ExitStatus ) ),\
        SLOT( finishedProcess( int, QProcess::ExitStatus ) ) );

#if defined( Q_OS_WIN )
    m_process->start( processPath, args, QIODevice::ReadWrite | QIODevice::Unbuffered );
    ret = m_process->waitForFinished( -1 ) ? 0 : 1;
#else
    m_process->setProgram( processPath, args );
    m_process->setNextOpenMode( QIODevice::ReadWrite | QIODevice::Unbuffered );
    m_process->start();
    QEventLoop loop;
    m_loop = &loop;
    ret = loop.exec( QEventLoop::WaitForMoreEvents );
    m_loop = 0;
#endif

    delete m_process;
    m_process = 0;

    return ret;
}

ThumbCreator::Flags Cb7Creator::flags() const
{
    return DrawFrame;
}

#include "cb7creator.moc"
