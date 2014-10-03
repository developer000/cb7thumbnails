/**
 *
 * cb7 Comic Book Thumbnailer for KDE 4 v0.2
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

// cb7creator.h

#ifndef COMIC_CREATOR_H
#define COMIC_CREATOR_H

#include <kio/thumbcreator.h>

#include <QtCore/QByteArray>
#include <QtCore/QEventLoop>
#include <QtCore/QStringList>
#include <QtGui/QImage>

#if defined( Q_OS_WIN )
    #include <QtCore/QProcess>
#else
    #include <kptyprocess.h>
#endif

class Cb7Creator : public QObject, public ThumbCreator
{
    Q_OBJECT
    public:
        Cb7Creator();
        ~Cb7Creator();
        virtual bool create( const QString &path, int width, int height, QImage &img );
        virtual Flags flags() const;

    private:
        void filterImages( QStringList &entries, const bool sensitive );
        int  startProcess( const QString processPath, QStringList args );

        // For 7z type files.
        	
        bool extract7zImage( QString path );
        
	bool is7zAvailable( QString &uzlPath );
        void get7zFileList( QStringList &entries, const QString path, const QString uzPath );
        
    private Q_SLOTS:
        void readProcessOut();
        void readProcessErr();
        void finishedProcess( int exitCode, QProcess::ExitStatus exitStatus );

    private:
        QImage *m_comicCover;
#if defined( Q_OS_WIN )
        QProcess *m_process;
#else
        KPtyProcess *m_process;
#endif
        QByteArray m_stdOut;
        QByteArray m_stdErr;
        QEventLoop *m_loop;
};

#endif
