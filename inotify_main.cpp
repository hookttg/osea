#include "qrsdet.h"		// For sample rate.
#include "time.h"
#include "string.h"
#include "win.h"
#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <sys/inotify.h>
#include <glog/logging.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <map>

#define EXAMPLE_USER "healthme"
#define EXAMPLE_PASS "Obama505"
#define EXAMPLE_DB "test"


#include "filter.h"
#include "config.h"
#include "boost/thread.hpp"
#include <boost/lockfree/queue.hpp>
#include <boost/filesystem/operations.hpp>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "watch.h"
#include "kstwo.h"

// monitor the IN_CREATE and IN_CLOSE_WRITE, IN_ATTRIB event
// when a new data file is created, the analyze thread is triggered by IN_CREATE event
// when a re-analyze is asked, java program will modify the time of original data file, hence a IN_ATTRIB event is triggered.
//there is an ending mark in each data file wich is 0x000 000 FEF EFE FE 000 000

using std::map;
using std::string;
using std::cout;
using std::endl;


#define FC filterCoefs::Instance()
// Keep going  while run == true, or, in other words, until user hits ctrl-c
static bool run = true;

void sig_callback(int sig) {
    run = false;
}

enum ACTION {
    CREATE, WRITE, CLOSE
};

class command {
public:
    int wd; //parent id, used to retrieve the path
    int name_id; //name id, should be unique
    ACTION oper;
};
const int THREAD_NUM = 8;
vector< boost::shared_ptr<boost::lockfree::queue<command> > > message_queues;
//using namespace ulib;
Watch watch;
//power of 2
#ifdef MYSQL_CONN
void multithread_task(int index,st_worker_thread_param *handles) {
    handles->driver->threadInit();

#else
void multithread_task(int index) {
#endif
    //ecg *p;
    command cmd;
    char path[1024];
    string current_dir;
    while (true) {
        if (!message_queues[index]->empty()) {
            
            message_queues[index]->pop(cmd);
            if (cmd.oper == CREATE) {
                printf("cmd CREATE!\n");
                /*current_dir = watch.get(cmd.wd);
                sprintf(path, "%s/%ld.dat", current_dir.c_str(), cmd.name_id);
                p = new ecg(path, CIRCULAR_LEN);
                p->finish();
                delete p;
                *///watch.erase(cmd.wd);

            } else if (cmd.oper == WRITE) {

                printf("cmd write!\n");
               current_dir = watch.get(cmd.wd);
                sprintf(path, "%s/%ld.dat", current_dir.c_str(), cmd.name_id);
                TESTRECORD line;
                //check the qrs between begin and end
                /*vector<int> begin,end;
                begin.push_back(12761472+76);
                end.push_back(12762240+45);
                vector<vector<int>> out;
                line.ReSearchQRS(path,begin,end,&out);*/
                //find the qrs and classify
                line.TestRecord(path);

            } else if (cmd.oper == CLOSE) {

                printf("cmd CLOSE!\n");

                // for future use;
    /*            current_dir = watch.get(cmd.wd);
                sprintf(path, "%ld", cmd.name_id);

                std::size_t last=current_dir.rfind('/');
                string date_name=current_dir.substr(last+1);
//                std::cout << current_dir << "last:" << last  << "date_name:" << date_name << std::endl;
                AF af(128);
//                conf::Instance()->Load(SYS_CONF);
//                conf::Instance()->LoadKS("/etc/healthme/dist.txt");
                af.update((char *)date_name.c_str(), path, (char *)"bsp");;
     */
            }

        } else {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
        }

    }
}

void add_sub_dir(int fd,int wd, const char *path) {

    struct dirent* dent;
    struct stat fstat;
    char full_name[_POSIX_PATH_MAX + 1];
    DIR* srcdir = opendir(path);
    int path_len = strlen(path);

    while ((dent = readdir(srcdir)) != NULL) {
        if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;
        strcpy(full_name, path);
        if (full_name[path_len - 1] != '/')
            strcat(full_name, "/");
        strcat(full_name, dent->d_name);
        if (stat(full_name, &fstat) < 0)
            continue;
        if (S_ISDIR(fstat.st_mode)) {

            int wd_sub = inotify_add_watch(fd, full_name, WATCH_FLAGS);
            watch.insert(wd, dent->d_name, wd_sub);
            LOG(INFO) << "adding monitoring directory:" << full_name;
        }
    }
    LOG(INFO).flush();
    closedir(srcdir);

}

bool dir_exists(const char* pzPath) {
    if (pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir(pzPath);

    if (pDir != NULL) {
        bExists = true;
        (void) closedir(pDir);
    }

    return bExists;
}

int main(int argc,char*argv[]) {

    google::InitGoogleLogging("HEALTHME");
  
    conf::Instance()->Load(SYS_CONF);
    conf::Instance()->LoadKS(DIST_CONF);
    //if (!FC->init(WAVE_FILTER)){
    //    LOG(ERROR)<<"No " << WAVE_FILTER ;
    //    return -1;
    //}
    // std::map used to keep track of wd (watch descriptors) and directory names
    // As directory creation events arrive, they are added to the Watch map.
    // Directory delete events should be (but currently aren't in this sample) handled the same way.


    // watch_set is used by select to wait until inotify returns some data to 
    // be read using non-blocking read.
    fd_set watch_set;

    char buffer[ EVENT_BUF_LEN ];
    string DATA_FILE_PATH;

    conf::Instance()->Get("read_path", DATA_FILE_PATH);
    const char *root = DATA_FILE_PATH.c_str();
    if (!dir_exists(root)) {
        LOG(ERROR) << "no read path:" << root << "!, please check " << SYS_CONF;
        return -1;
    }
    conf::Instance()->Get("write_path", DATA_FILE_PATH);
    if (!dir_exists(DATA_FILE_PATH.c_str())) {
        LOG(ERROR) << "no write path:" << DATA_FILE_PATH << "!, please check " << SYS_CONF;
        return -1;
    }
#ifdef MYSQL_CONN
    sql::Driver *driver;
    boost::scoped_ptr< sql::Connection > con;
    string url="localhost";
    string user="healthme";
    string pass="Obama505";
    string database="healthme";
    try {
        driver = sql::mysql::get_driver_instance();

        /* Using the Driver to create a connection */
        con.reset(driver->connect(url, user, pass));
        con->setSchema(database);


    } catch (sql::SQLException &e) {

        LOG(ERROR) << "# ERR: SQLException in " << __FILE__;
        LOG(ERROR) << " on line " << __LINE__;
        LOG(ERROR) << "# ERR: " << e.what();
        LOG(ERROR) << " (MySQL error code: " << e.getErrorCode();
        LOG(ERROR) << ", SQLState: " << e.getSQLState() << " )";

        return EXIT_FAILURE;

    } catch (std::runtime_error &e) {

        LOG(ERROR) << "# ERR: runtime_error in " << __FILE__;
        LOG(ERROR) << " on line " << __LINE__;
        LOG(ERROR) << "# ERR: " << e.what();


        return EXIT_FAILURE;
    }

    struct st_worker_thread_param *param = new st_worker_thread_param;

    param->driver = driver;
    param->con = con.get();
#endif
    string current_dir, new_dir;
    int total_file_events = 0;
    int total_dir_events = 0;

    // Call sig_callback if user hits ctrl-c
    signal(SIGINT, sig_callback);
    signal(SIGTERM, sig_callback);    
    // creating the INOTIFY instance
    // inotify_init1 not available with older kernels, consequently inotify reads block.
    // inotify_init1 allows directory events to complete immediately, avoiding buffering delays. In practice,
    // this significantly improves monotiring of newly created subdirectories.

    int fd = inotify_init1(IN_NONBLOCK);
    // checking for error
    if (fd < 0) {
        LOG(ERROR) << "inotify_init failed";
        return -1;
    }

    // use select watch list for non-blocking inotify read
    FD_ZERO(&watch_set);
    FD_SET(fd, &watch_set);

    int wd = inotify_add_watch(fd, root, WATCH_FLAGS);
    printf("root=:%s!\n",root);
    // add wd and directory name to Watch map
    watch.insert(-1, root, wd);

    char latest_sub_dir[_POSIX_PATH_MAX + 1];
    char latest_sub_name[_POSIX_PATH_MAX + 1];
    //get_latest_sub_dir(root, latest_sub_dir, latest_sub_name);
    //add the latest subdirectory into watch in 
    // for example 2014_10_09 , 2014_10_10, then 2014_10_10 was added to monitor, 
    // because only the latest directory where the data can be written.
//    int wd_sub = inotify_add_watch(fd, latest_sub_dir, WATCH_FLAGS);
//    watch.insert(wd, latest_sub_name, wd_sub);
    add_sub_dir(fd,wd, root);

    for (int i = 0; i < THREAD_NUM; i++) {

        boost::shared_ptr<boost::lockfree::queue<command> > queue1(new boost::lockfree::queue<command>(100));
        message_queues.push_back(queue1);

    }
    boost::thread_group threads;
    for (int i = 0; i < THREAD_NUM; i++) {
#ifdef MYSQL_CONN
        threads.create_thread(boost::bind(&multithread_task, i,param));
#else
        threads.create_thread(boost::bind(&multithread_task, i));
#endif
    }
    LOG(INFO)<<argv[0] << " started!";
    cout<< argv[0] << " started!\n";

    // Continue until run == false. See signal and sig_callback above.
    while (run) {
        // select waits until inotify has 1 or more events.
        // select syntax is beyond the scope of this sample but, don't worry, the fd+1 is correct:
        // select needs the the highest fd (+1) as the first parameter.
        //root = "/opt/ecgData";
        select(fd + 1, &watch_set, NULL, NULL, NULL);

        // Read event(s) from non-blocking inotify fd (non-blocking specified in inotify_init1 above).
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            //LOG(ERROR) << "read "  << buffer;
            continue;
        }

        // Loop through event buffer
        for (int i = 0; i < length;) {
            struct inotify_event *event = (struct inotify_event *) &buffer[ i ];
            // Never actually seen this
            if (event->wd == -1) {
                LOG(ERROR) << ("Overflow\n");
            }
            // Never seen this either		
            if (event->mask & IN_Q_OVERFLOW) {
                LOG(ERROR) << ("Overflow\n");
            }

            if (event->mask & IN_IGNORED) {
                LOG(ERROR) << ("IN_IGNORED\n");
            }
            if (event->mask & IN_CREATE) {
                current_dir = watch.get(event->wd);
                if (event->mask & IN_ISDIR) {
                    new_dir = current_dir + "/" + event->name;
                    //int pre_wd = wd_sub;
                    int wd_sub = inotify_add_watch(fd, new_dir.c_str(), WATCH_FLAGS);
                    watch.insert(event->wd, event->name, wd_sub);
                    strcpy(latest_sub_name, event->name);
                    total_dir_events++;
                    LOG(INFO)<<"New directory "<< new_dir <<" is created and added into monitor directory.";
                } else {
                    total_file_events++;
                    printf("created!\n");
                    command cmd;    
                    char suffix[10];  
                    //LOG(INFO)<<"New file "<< current_dir<<"/"<< event->name<<" created.";                       
                    sscanf(event->name, "%ld.%s", &cmd.name_id,suffix);
                    if (cmd.name_id!=0 && strcmp(suffix,"dat")==0) {
                        LOG(INFO)<<"New file "<< current_dir<<"/"<< event->name<<" created.";                        
                        cmd.wd = event->wd;
                        cmd.oper = CREATE;
                        int index = cmd.name_id % THREAD_NUM;
                        message_queues[index]->push(cmd);
                        LOG(INFO)<<"pushed into "<< index <<" queue.";                         
                    }

                }
            }  else if (event->mask & IN_CLOSE_WRITE) {

                current_dir = watch.get(event->wd);
                command cmd;
                char suffix[10];
                sscanf(event->name, "%ld.%s", &cmd.name_id,suffix);
                if (cmd.name_id != 0 & strcmp(suffix,"dat")==0) {
                    cmd.wd = event->wd;
                    cmd.oper = WRITE;
                    int index = cmd.name_id % THREAD_NUM;
                    message_queues[index]->push(cmd);
                }

                printf("closed!\n");
//                printf("root=:%s!\n",root);
                total_file_events--;
                LOG(INFO)<<"File "<<current_dir<<"/"<<event->name <<" closed.";



            } else if (event->mask & IN_ATTRIB) {

                current_dir = watch.get(event->wd);
                total_file_events++;
                printf("IN_ATTRIB!\n");
//                printf("root=:%s!\n",root);
                LOG(INFO) << " file " << current_dir << "/" << event->name << " attrib is modified.";
//                command cmd;
//                char suffix[10];
//                sscanf(event->name, "%ld.%s", &cmd.name_id,suffix);
//                if (cmd.name_id != 0 & strcmp(suffix,"dat")==0) {
//                    cmd.wd = event->wd;
//                    cmd.oper = WRITE;
//                    int index = cmd.name_id % THREAD_NUM;
//                    message_queues[index]->push(cmd);
//                }
//                if (cmd.name_id != 0 & strcmp(suffix,"cnf")==0) { // this for af analysis
//                    cmd.wd = event->wd;
//                    cmd.oper = CLOSE;
//                    int index = cmd.name_id % THREAD_NUM;
//                    message_queues[index]->push(cmd);
//                }

            }            

            i += EVENT_SIZE + event->len;
        }
    }

    // Cleanup
    printf("cleaning up\n");
    cout << "total dir events = " << total_dir_events << ", total file events = " << total_file_events << endl;
    watch.stats();
    watch.cleanup(fd);
    watch.stats();
    close(fd);
    LOG(INFO).flush();    
    google::ShutdownGoogleLogging();    
    fflush(stdout);

    for (int i = 0; i < message_queues.size(); i++) {
        if (!message_queues[i]->empty()) {
            command cmd;

            message_queues[i]->pop(cmd);
            std::cout << "name:" << cmd.name_id << std::endl;
        }
    }
}

