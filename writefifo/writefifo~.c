#define _GNU_SOURCE

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "m_pd.h"

#define FIFO_NAME "/home/mike/Music/fifo"

static t_class *writefifo_tilde_class;

typedef struct _writefifo_tilde {
  t_object x_obj; // mandatory object
  t_sample f;     // dummy to enable signal on first inlet
  t_inlet *x_in2; // second signal inlet
  int writefd;    // fifo file descriptor
} t_writefifo_tilde;

t_int *writefifo_tilde_perform(t_int *w) {
  t_writefifo_tilde *x = (t_writefifo_tilde *)(w[1]);
  t_sample *in1 = (t_sample *)(w[2]);
  t_sample *in2 = (t_sample *)(w[3]);
  int n = (int)(w[4]);

  for (int i = 0; i < n; i++) {
    // do work here
  }

  return (w + 5); // return pointer to next dsp object dataspace
}

void writefifo_tilde_dsp(t_writefifo_tilde *x, t_signal **sp) {
  /* register the writefifo_perform() function with DSP-tree */
  dsp_add(writefifo_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec,
          sp[0]->s_n);
}

void writefifo_tilde_free(t_writefifo_tilde *x) {
  inlet_free(x->x_in2);
  close(x->writefd); // also maybe delete fifo?
}

void *writefifo_tilde_new(void) {
  t_writefifo_tilde *x = (t_writefifo_tilde *)pd_new(writefifo_tilde_class);
  x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  struct stat st;
  if (!stat(FIFO_NAME, &st) && S_ISFIFO(st.st_mode)) {
    /* fifo already exists */
    post("fifo %s already exists", FIFO_NAME);
  } else {
    /* make fifo */
    post("creating fifo %s", FIFO_NAME);
    mkfifo(FIFO_NAME, 0666);
  }

  /* open fifo for read first so open for write succeeds */
  int readfd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
  x->writefd = open(FIFO_NAME, O_WRONLY);
  close(readfd);

  /* set fifo size to minimize latency */
  fcntl(x->writefd, F_SETPIPE_SZ, 0x2000);
  return (void *)x;
}

void writefifo_tilde_setup(void) {
  writefifo_tilde_class =
      class_new(gensym("writefifo~"),             // object name
                (t_newmethod)writefifo_tilde_new, // constructor
                (t_method)writefifo_tilde_free,   // destructor
                sizeof(t_writefifo_tilde),        // dataspace size
                CLASS_DEFAULT,                    // normal object
                0);                               // no arguments

  /* call writefifo_dsp() when audio-engine starts */
  class_addmethod(writefifo_tilde_class, (t_method)writefifo_tilde_dsp,
                  gensym("dsp"), 0);

  /* use first inlet as a signal */
  CLASS_MAINSIGNALIN(writefifo_tilde_class, t_writefifo_tilde, f);
}
