#ifndef _COMMAND_LISTENER_H__
#define _COMMAND_LISTENER_H__

#include <sysutils/FrameworkListener.h>
#include <sysutils/FrameworkCommand.h>

class CommandListener : public FrameworkListener {
public:
    CommandListener();
    virtual ~CommandListener() {}
};

#endif // _COMMANDLISTENER_H__
