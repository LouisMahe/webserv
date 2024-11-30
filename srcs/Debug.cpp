#include <ILogger.hpp>
#include <Host.hpp>

void	Host::printLocationsDebug(void) {
	for (std::list<Location>::const_iterator it = _locations.begin();
			it != _locations.end(); it++) {
		LOGD("%Lo", &*it);
	}
}
