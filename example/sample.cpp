#include <iostream>
#include <twitch_bot.hpp>

int main()
{
	std::string user = "<Username>";
	// Obtain password from: https://twitchapps.com/tmi/
	std::string pass = "<Password>";
	
	std::string channel = "<ChannelName>";

	twb::Bot myBot(user, pass);

	myBot.BindOnReceiveMessageCallback([&](const std::string& senderUsername, const std::string& senderMessage) {
		std::cout << senderUsername << ": " << senderMessage << std::endl;
		myBot.Message("Hi @" + senderUsername);
	});

	myBot.BindOnJoinChannelChatCallback([&]() { myBot.Message("Hello everyone, I just joined the chat!"); });
	myBot.ConnectTo(channel);

	return 0;
}