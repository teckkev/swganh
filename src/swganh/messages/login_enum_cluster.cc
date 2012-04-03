// This file is part of SWGANH which is released under GPL v2.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "login_enum_cluster.h"

#include "swganh/login/login_client.h"

using namespace swganh::login;
using namespace swganh::messages;
using namespace std;

LoginEnumCluster swganh::messages::BuildLoginEnumCluster(
    shared_ptr<LoginClient> login_client, 
    const vector<GalaxyStatus>& galaxy_status) 
{
	LoginEnumCluster message;
	message.max_account_chars = 2;
	
	std::for_each(galaxy_status.begin(), galaxy_status.end(), [&message] (const GalaxyStatus& status) {
		Cluster cluster;
		cluster.server_id = status.galaxy_id;
		cluster.server_name = status.name;
		cluster.distance = status.distance;
		
		message.servers.push_back(cluster);
	});
	
	return message;
}
