/* 
 * File:   watch.h
 * Author: guiwen
 *
 * Created on December 3, 2014, 2:53 PM
 */

#ifndef WATCH_H
#define	WATCH_H

#include "ulib/os_thread.h"
#include "ulib/util_timer.h"
#include "ulib/hash_chain_r.h"
#include "ulib/math_rand_prot.h"
// Watch class keeps track of watch descriptors (wd), parent watch descriptors (pd), and names (from event->name).
// The class provides some helpers for inotify, primarily to enable recursive monitoring:
// 1. To add a watch (inotify_add_watch), a complete path is needed, but events only provide file/dir name with no path.
// 2. Delete events provide parent watch descriptor and file/dir name, but removing the watch (infotify_rm_watch) needs a wd.
//
#define EVENT_SIZE          ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN       ( 1024 * ( EVENT_SIZE + NAME_MAX + 1) )
#define WATCH_FLAGS         ( IN_CREATE | IN_CLOSE_WRITE |IN_ATTRIB)
class Watch {

    struct wd_elem {
        int pd;
        string name;
    };
    ulib::chain_hash_map_r<int, wd_elem> watch;
    ulib::chain_hash_map_r<int, wd_elem*> watch1;    

public:
    // Insert event information, used to create new watch, into Watch object.
    Watch():watch(100,10),watch1(100,10)
    {
       
    }

    void insert(int pd, const string &name, int wd) {
        wd_elem elem = {pd, name};
        wd_elem *pelem=new wd_elem();
        pelem->name=name;
        pelem->pd=pd;
        
        //watch[wd] = elem;
        watch1.insert(wd,pelem);
    }

    // Given a watch descriptor, return the full directory name as string. Recurses up parent WDs to assemble name, 
    // an idea borrowed from Windows change journals.

//    string get(int wd) {
//        const wd_elem &elem = watch[wd];
//        return elem.pd == -1 ? elem.name : this->get(elem.pd) + "/" + elem.name;
//    }
    string get(int wd) {
        wd_elem *elem = watch1[wd];
        return elem->pd == -1 ? elem->name : this->get(elem->pd) + "/" + elem->name;
    }
    void erase(int wd) {
        wd_elem *elem = watch1[wd];
        watch1.erase(wd);        
        delete elem;


    }    
    void cleanup(int fd) {
        for (ulib::chain_hash_map_r<int, wd_elem*>::iterator wi = watch1.begin(); wi != watch1.end(); wi++) {
            inotify_rm_watch(fd, wi.key());
            delete wi.value();
            watch1.erase(wi);
        }
    }

    void stats() {
        cout << "number of watches=" << watch1.size() << endl;
    }
};

#endif	/* WATCH_H */

