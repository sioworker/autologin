#ifndef AUTOLOGIN_CONFIG_H
#define AUTOLOGIN_CONFIG_H

#define DEFAULT_CONF "/etc/autologin.conf"

struct config {
	char user[256];
	int relogin;
	char *script;
};

/// Parse the config at DEFAULT_CONF
void parse_config(const char *path, struct config *cfg);

#endif
