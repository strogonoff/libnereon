/*
 * Copyright (c) 2018, [Ribose Inc](https://www.ribose.com).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>

#include "rvd.nos.h"
#include "common.h"
#include "nereon.h"

#define NOC_CONFIG_FILE                     "rvd.noc"

struct rvd_options {
	char *config_fpath;
	bool go_daemon;

	bool check_config;
	bool print_version;
	bool print_help;

	char *ovpn_bin_path;
	bool ovpn_root_check;
	bool ovpn_use_scripts;

	int allowed_uid;
	bool restrict_cmd_sock;

	char *log_dir_path;
	char *vpn_config_dir;
};

/*
 * print rvd config options
 */

static void print_config_options(struct rvd_options *opt)
{
	fprintf(stderr, "config_fpath => %s\n", opt->config_fpath);
	fprintf(stderr, "go_daemon => %s\n", opt->go_daemon ? "true" : "false");
	fprintf(stderr, "check_config => %s\n", opt->check_config ? "true" : "false");
	fprintf(stderr, "print_version => %s\n", opt->print_version ? "true" : "false");
	fprintf(stderr, "print_help => %s\n", opt->print_help ? "true" : "false");
	fprintf(stderr, "ovpn_bin_path => %s\n", opt->ovpn_bin_path);
	fprintf(stderr, "ovpn_root_check => %s\n", opt->ovpn_root_check ? "true" : "false");
	fprintf(stderr, "ovpn_use_scripts => %s\n", opt->ovpn_use_scripts ? "true" : "false");
	fprintf(stderr, "allowed_uid => %d\n", opt->allowed_uid);
	fprintf(stderr, "restrict_cmd_sock => %s\n", opt->restrict_cmd_sock ? "true" : "false");
	fprintf(stderr, "log_dir_path => %s\n", opt->log_dir_path);
	fprintf(stderr, "vpn_config_dir => %s\n", opt->vpn_config_dir);
}

/*
 * main function
 */

int main(int argc, char *argv[])
{
	nereon_ctx_t ctx;
	int ret;
	bool require_exit = false;

	struct rvd_options rvd_opts;

	struct nereon_config_option cfg_opts[] = {
		{"config_file", NEREON_TYPE_CONFIG, false, NULL, &rvd_opts.config_fpath},
		{"go_daemon", NEREON_TYPE_BOOL, false, NULL, &rvd_opts.go_daemon},
		{"check_config", NEREON_TYPE_BOOL, false, NULL, &rvd_opts.check_config},
		{"print_version", NEREON_TYPE_BOOL, false, NULL, &rvd_opts.print_version},
		{"helper", NEREON_TYPE_BOOL, false, NULL, &rvd_opts.print_help},
		{"openvpn_bin", NEREON_TYPE_STRING, true, NULL, &rvd_opts.ovpn_bin_path},
		{"openvpn_root_check", NEREON_TYPE_BOOL, false, NULL, &rvd_opts.ovpn_root_check},
		{"openvpn_updown_scripts", NEREON_TYPE_BOOL, false, NULL, &rvd_opts.ovpn_use_scripts},
		{"user_id", NEREON_TYPE_INT, true, NULL, &rvd_opts.allowed_uid},
		{"restrict_socket", NEREON_TYPE_BOOL, false, NULL, &rvd_opts.restrict_cmd_sock},
		{"log_directory", NEREON_TYPE_STRING, true, NULL, &rvd_opts.log_dir_path},
		{"vpn_config_paths", NEREON_TYPE_STRING, true, NULL, &rvd_opts.vpn_config_dir}
	};

	int rvd_opts_count;

	/* initialize nereon context */
	ret = nereon_ctx_init(&ctx, get_rvd_nos_cfg());
	if (ret != 0) {
		fprintf(stderr, "Could not initialize nereon context(err:%s)\n", nereon_get_errmsg());
		exit(1);
	}

	fprintf(stdout, "libnereon version_info: %s\n", nereon_get_version_info());

	/* print command line usage */
	ret = nereon_parse_cmdline(&ctx, argc, argv, &require_exit);
	if (ret != 0 || require_exit) {
		if (ret != 0)
			fprintf(stderr, "Failed to parse command line(err:%s)\n", nereon_get_errmsg());

		nereon_print_usage(&ctx);
		nereon_ctx_finalize(&ctx);

		exit(ret);
	}

	memset(&rvd_opts, 0, sizeof(struct rvd_options));
	if (nereon_parse_config_file(&ctx, NOC_CONFIG_FILE) != 0) {
		fprintf(stderr, "Could not parse NOC configuration(err:%s)\n", nereon_get_errmsg());
		nereon_ctx_finalize(&ctx);

		exit(1);
	}

	ret = nereon_get_config_options(&ctx, cfg_opts);
	if (ret == 0) {
		print_config_options(&rvd_opts);
	} else {
		fprintf(stderr, "Failed to get configuration options(err:%s)\n", nereon_get_errmsg());
	}

	/* finalize nereon context */
	nereon_ctx_finalize(&ctx);

	return ret;
}
