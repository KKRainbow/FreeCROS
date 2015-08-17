#pragma once

#include"Global.h"
#include"Multiboot.h"
#include"ExecutableFormat.h"

class ServerLoader {
SINGLETON_H(ServerLoader)

public:
    void LoadModules();
};
