#include <iostream>
#include <twb/twitch_bot.hpp>

int main(int argc, char** argv)
{
	if (argc < 4) 
	{ 
		std::cerr << "3 parameters required, " << (argc-1) << " were given." << std::endl;
		std::cerr << "Usage: " << argv[0] << " user password channel" << std::endl;
		return 1;
	}
	std::string user = argv[1];
	// Obtain password from: https://twitchapps.com/tmi/
	std::string pass = argv[2];
	
	std::string channel = argv[3];

	twb::Bot myBot(user, pass);

	myBot.BindOnReceiveMessageCallback([&](const std::string& senderUsername, const std::string& senderMessage) {
		std::cout << senderUsername << ": " << senderMessage << std::endl;
		myBot.Message("Hi @" + senderUsername);
	});

	myBot.BindOnJoinChannelChatCallback([&]() { myBot.Message("Hello everyone, I just joined the chat!"); });
	myBot.ConnectTo(channel);

	return 0;
}