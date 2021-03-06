#include "view.h"

#include "config.h"
#include "unsubscribe.h"
#include "util.h"

#include <Wt/WEnvironment>
#include <Wt/WServer>

#include <signal.h>

std::string getValueFromEnv(const Wt::WEnvironment& env,
                            const std::string &key,
                            const std::string &defaultValue)
{
	std::vector<std::string> param = env.getParameterValues(key);
	if (param.size() > 0) {
		return param[0];
	} else
		return defaultValue;
}

Wt::WApplication *createApplication(const Wt::WEnvironment& env,
                                    Wt::WServer &server)
{
	std::string thread = getValueFromEnv(env, "url", "default");
	std::string unsub = getValueFromEnv(env, "unsub", "0");

	if (unsub == "1")
		return new Unsubscribe(env, server, thread);
	else
		return new View(env, server, thread);
}

int main(int argc, char **argv)
{
	Config* conf = Config::getConfig();
	if(conf->readConfigFile() == false) {
		return 1;
	}
	std::cout << "Successfully read configuration file '"
	             + conf->getConfigFileName() + "'\n";

	try {
		Wt::WServer server(argv[0]);

		server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

		server.addEntryPoint(Wt::Application,
		                     boost::bind(createApplication, _1,
		                                 boost::ref(server)));

		/* change the CWD to the binary's folder */
		chdir(getExeDirectory().c_str());

		if (server.start()) {
			int sig = Wt::WServer::waitForShutdown(argv[0]);

			std::cerr << "Shutdown (signal = " << sig << ")" << std::endl;
			server.stop();

			if (sig == SIGHUP)
				Wt::WServer::restart(argc, argv, environ);
		}
	} catch (Wt::WServer::Exception& e) {
		std::cerr << e.what() << "\n";
		return 1;
	} catch (std::exception& e) {
		std::cerr << "exception: " << e.what() << "\n";
		return 1;
	}
}
