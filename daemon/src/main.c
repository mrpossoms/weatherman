#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <errno.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


#include "../../firmware/weatherman/protocol.h"
#include "html.h"

struct 
{
	int running;
	int sock;
} state;

struct
{
	char* log_file_path;
} CONF;


#define ARR_LEN(a) (sizeof(a) / sizeof(a[0]))

void sig_handler(int sig)
{
	switch(sig)
	{
		case SIGTERM:
			state.running = 0;
			break;
		
	}
}

int socket_init(unsigned short port)
{
	int enable = 1;
	struct sockaddr_in rcv_addr = {};
	socklen_t addr_len = sizeof(rcv_addr);

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (sock < 0)
	{
		fprintf(stderr, "socket() failed: %s", strerror(errno));
		exit(1);
	}

	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const void*)&enable, sizeof(enable)))
	{
		fprintf(stderr, "setsockopt() failed: %s", strerror(errno));
		exit(1);	
	}

	
	/* We bind it to broadcast addr on the given port */
	rcv_addr.sin_family = AF_INET;
	rcv_addr.sin_port = htons(port);
	rcv_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *) &rcv_addr, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "bind() failed: %s", strerror(errno));
		exit(1);
	}

	return sock;
}

void write_col_l(int fd, long val)
{
	char buf[128];
	int len = snprintf(buf, sizeof(buf), "%ld", val);
	write(fd, buf, len);
	write(fd, ",", 1);
}

void write_col_d(int fd, double val)
{
	char buf[128];
	int len = snprintf(buf, sizeof(buf), "%f", val);
	write(fd, buf, len);
	write(fd, ",", 1);
}

void write_col_str(int fd, const char* val)
{
	write(fd, val, strlen(val));
	write(fd, ",", 1);
}

int open_log(int num_cols, const char* cols[])
{
	unsigned day = time(NULL) / (24 * 3600);
	char path[256];

	snprintf(path, sizeof(path), "%s/%d.csv", CONF.log_file_path, day);

	int log_fd = open(path, O_CREAT | O_APPEND | O_WRONLY, 0655);

	if (log_fd < 0)
	{
		fprintf(stderr, "open() failed: %s", strerror(errno));
		exit(1);
	}

	off_t log_len = lseek(log_fd, 0, SEEK_END);
	lseek(log_fd, 0, SEEK_SET);

	// if the log is new, write a header
	if (log_len == 0)
	{
		for (unsigned i = 0; i < num_cols; i++)
		{
			write_col_str(log_fd, cols[i]);
		}
		write(log_fd, "\n", 1);
	}

	return log_fd;
}

void log_measurements(int sock)
{
	char buf[1024];
	int bytes_read = read(sock, buf, sizeof(buf));

	if (bytes_read > 0)
	{
		void* parse_ptr = (void*)buf;

		char* cols[] = {
			"timestamp",
			"rssi:db",
			"relative humidity:%",
			"temperature:C",
		};

		measurement_t* measurements[] = {
			NULL,
			NULL,
			NULL,
			NULL,
		};

		char* meas_map[] = {
			NULL,
			NULL,
			"HUMI",
			"TEMP"
		};

		int log_fd = open_log(ARR_LEN(cols), (const char**)cols);

		header_t* hdr = (header_t*)parse_ptr;
		parse_ptr += sizeof(header_t);

		write_col_l(log_fd, time(NULL)); // write timestamp
		write_col_l(log_fd, hdr->rssi); // write rssi

		for (unsigned i = 0; i < hdr->measurement_count; i++)
		{
			measurement_t* m = (measurement_t*)parse_ptr;
			parse_ptr += sizeof(measurement_t);

			for (unsigned j = 0; j < sizeof(meas_map) / sizeof(char*); j++)
			{
				if (NULL == meas_map[j]) { continue; }

				if (strncmp(m->sensor, meas_map[j], sizeof(m->sensor)) == 0)
				{
					measurements[j] = m;
				}
			}
		}

		for (unsigned i = 2; i < ARR_LEN(measurements); i++)
		{
			write_col_d(log_fd, measurements[i]->value);
		}

		write(log_fd, "\n", 1);

		close(log_fd);
	}
}

int main(int argc, const char* argv[])
{
	signal(SIGTERM, sig_handler);

	state.running = 1;
	state.sock = socket_init(31337);

	CONF.log_file_path = "/var/weatherman";

	key_value_t kvps[] = {
		{
			.key = "TEMP",
			.value = "30"
		}
	};

	view(kvps);

	while (state.running)
	{
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(state.sock, &rfds);

		if (select(state.sock + 1, &rfds, NULL, NULL, NULL) > 0)
		{
			log_measurements(state.sock);
		}
	}

	close(state.sock);
	printf("shutdown\n");

	return 0;
}
