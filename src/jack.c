#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <jack/jack.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "nanovg.h"

#include "orb.h"
#include "jack.h"

#define BUFSIZE 2048


typedef struct {
    jack_port_t **output_port;
    jack_port_t *input_port;
    jack_client_t **client;
    const char **ports;
    orb_data *orb;
    int run;
} jack_data;

static jack_data jd;

void error(char *msg) {
  perror(msg);
  exit(1);
}

static int jack_cb(jack_nframes_t nframes, void *arg)
{
    int chan;
    jack_data *jack = arg;
    orb_data *orb = jack->orb;

    if(jack->run == 0) return 0;
    jack_default_audio_sample_t  *out[2];

    for(chan = 0; chan < 2; chan++)
        out[chan] = jack_port_get_buffer (jack->output_port[chan], nframes);

    orb_audio(orb, out, nframes);

    return 0;
}

static void jack_shutdown (void *arg) {
    exit (1);
}

void orb_start_jack(orb_data *orb, int sr)
{
    const char *server_name = NULL;
    int chan;
    jack_options_t options = JackNullOption;
    jack_status_t status;

    const char **ports = jd.ports;
    char client_name[256];

    jd.orb = orb;
    strncpy(client_name, "orb", 256);

    jd.output_port = malloc(sizeof(jack_port_t *) * 2);
    jd.client = malloc(sizeof(jack_client_t *));
   
    jd.client[0] = jack_client_open (client_name, options, 
        &status, server_name);
    jd.run = 0;

    if (jd.client[0] == NULL) {
        fprintf (stderr, "jack_client_open() failed, "
             "status = 0x%2.0x\n", status);
        if (status & JackServerFailed) {
            fprintf (stderr, "Unable to connect to JACK server\n");
        }
        exit (1);
    }

    if (status & JackServerStarted) {
        fprintf (stderr, "JACK server started\n");
    }

    if (status & JackNameNotUnique) {
        /* client_name = jack_get_client_name(jd.client[0]); */
        fprintf (stderr, "unique name `%s' assigned\n", client_name);
    }
    jack_set_process_callback (jd.client[0], jack_cb, &jd);
    jack_on_shutdown (jd.client[0], jack_shutdown, 0);

    char chan_name[50];

    jd.input_port = jack_port_register(jd.client[0], "sp_input",
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsInput, 0);

    for(chan = 0; chan < 2; chan++) {
        sprintf(chan_name, "output_%d", chan);
        jd.output_port[chan] = jack_port_register (jd.client[0], chan_name,
                          JACK_DEFAULT_AUDIO_TYPE,
                          JackPortIsOutput, chan);

        if (jd.output_port[chan] == NULL) {
            fprintf(stderr, "no more JACK ports available\n");
            exit (1);
        }

        if (jack_activate (jd.client[0])) {
            fprintf (stderr, "cannot activate client");
            exit (1);
        }
    }
    ports = jack_get_ports (jd.client[0], NULL, NULL,
                JackPortIsPhysical|JackPortIsInput);
    if (ports == NULL) {
        fprintf(stderr, "no physical playback ports\n");
        exit (1);
    }
    for(chan = 0; chan < 2; chan++) {
        if (jack_connect (jd.client[0], jack_port_name (jd.output_port[chan]), ports[chan])) {
            fprintf (stderr, "cannot connect output ports\n");
        }
    }

    jd.run = 1;
}

void orb_stop_jack(orb_data *orb)
{
    jd.run = 0;
    free (jd.ports);
    jack_client_close(jd.client[0]);
    free(jd.output_port);
    free(jd.client);
}
