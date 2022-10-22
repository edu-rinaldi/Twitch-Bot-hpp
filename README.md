# TwitchBot - Header only library for creating twitch bots
Header only library for creating custom twitch bots. In order to use this library you need:

- C++ 20
- To include `twitch_bot.hpp`
- Create a `twb::Bot` object and bind callback functions you want to use.

**Example:**

```cpp
#include <twitch_bot.hpp>

int main(int argc, char** argv)
{
    const char* user        = "edu_rinaldi";
    const char* password    = "<Password>";
    const char* channel     = "edu_rinaldi";
    // Create the bot
    twb::Bot myBot(user, password);

    // Bind functions
    myBot.BindOnReceiveMessageCallback([&](const std::string& senderUsername, const std::string& senderMessage) 
    { 
        std::cout << senderUsername << ": " << senderMessage << std::endl;
        myBot.Message("Hi @" + senderUsername); 
    });

    myBot.BindOnJoinChannelChatCallback([&]() { myBot.Message("Hello everyone, I just joined the chat!"); });

    // Connect and join channel's chat
    myBot.Connect(channel);
}
```

**Execution:**

![](images/1.PNG)
![](images/2.PNG)

## Callback functions
At the moment this library only support (**few**) callback functions for following events:

1. Bot join the chat, using `twb::Bot::BindOnJoinChannelChatCallback`.
2. A user send a message in channel chat, using `twb::Bot::BindOnReceiveMessageCallback`.

# Build example
I've prepared an example that you can use in order to start working on your awesome bot. To build it you can use *CMake*, for example as follows:

1. `mkdir build`
2. `cd build`
3. `cmake ..`

Or just use *CMake GUI*.

# Compatibility and test
This library compilation has been tested on following compilers:
- **Windows:** MSVC >= 2019
- **Linux:** GCC Version 9.3.0

# Contribute
Contributions are welcome, here's a list of possible features to work on:
* New events to which can be binded callback functions
* Additional `IRCMessage`(s)
* `IRCMessage` built from string (e.g.: `"PING :tmi.twitch.tv"` builds a `IRCMessage{.type = PING, .message = ":tmi.twitch.tv"}`)
* Better interface
* Test on *MacOS* and *Linux*