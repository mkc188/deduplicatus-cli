//
//  FileTransfer.h
//  deduplicatus-cli
//
//  Created by Ka Chun Mok on 11/4/15.
//  Copyright (c) 2015 Peter Lam. All rights reserved.
//

#ifndef __deduplicatus_cli__FileTransfer__
#define __deduplicatus_cli__FileTransfer__

#include <stdio.h>
#include <string>

using namespace std;

class FileTransfer {
public:
    FileTransfer();

    void uploadDropbox(string, string);

private:
};

#endif /* defined(__deduplicatus_cli__FileTransfer__) */
