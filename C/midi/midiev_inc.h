#ifndef __MIDIEV_INC_H
#define __MIDIEV_INC_H

#include "tool/tool.h"

/* Midi event list */
enum {
  NOTEOFF = 8,
  NOTEON,
  KEYAFTERTOUCH,
  CONTROLCHANGE,
  PROGRAMCHANGE,
  CHANNELAFTERTOUCH,
  PITCHWHEELCHANGE
};

typedef struct
{
  byte_t	type, chan;
  union
  {
    struct { byte_t num,val; }		note;
    struct { byte_t num,val; }		aftertouch;
    struct { byte_t num,val; }		ctrl;
    byte_t				prg_chg;
    byte_t				chan_aftertouch;
    struct { byte_t Lval,Hval; }	pitchbend;
  }		event;

  /* raw midi value without deltatime
     /!\ caution with one param or two param event */
  /* byte_t	raw[3]; */
}		midicev_t;

#define midicmd_to_str(cmd) ((cmd) == NOTEOFF ? "NOTEOFF" :		\
			     (cmd) == NOTEON ? "NOTEON" :		\
			     (cmd) == KEYAFTERTOUCH ? "KEYAFTERTOUCH" :	\
			     (cmd) == CONTROLCHANGE ? "CONTROLCHANGE" :	\
			     (cmd) == PROGRAMCHANGE ? "PROGRAMCHANGE" :	\
			     (cmd) == CHANNELAFTERTOUCH ? "CHANNELAFTERTOUCH" :	\
			     (cmd) == PITCHWHEELCHANGE ? "PITCHWHEELCHANGE" : \
			     "UNKNOWN")

/* #define midievtype_to_str(buf) ((buf) >> 4 == NOTEOFF ? "NOTEOFF" :	\ */
/* 				(buf) >> 4 == NOTEON ? "NOTEON" :	\ */
/* 				(buf) >> 4 == KEYAFTERTOUCH ? "KEYAFTERTOUCH" :	\ */
/* 				(buf) >> 4 == CONTROLCHANGE ? "CONTROLCHANGE" :	\ */
/* 				(buf) >> 4 == PROGRAMCHANGE ? "PROGRAMCHANGE" :	\ */
/* 				(buf) >> 4 == CHANNELAFTERTOUCH ? "CHANNELAFTERTOUCH" :	\ */
/* 				(buf) >> 4 == PITCHWHEELCHANGE ? "PITCHWHEELCHANGE" : \ */
/* 				(buf) == 0xFF ? "META-EVENT" :		\ */
/* 				"UNKNOWN")d */


/* Meta event list */
#define ME_SEQUENCENUMBER       0x00
#define ME_TEXTEVENT            0x01
#define ME_COPYRIGHTNOTICE      0x02
#define ME_NAME                 0x03
#define ME_CUEPOINT             0x07
#define ME_ENDOFTRACK           0x2F
#define ME_SETTEMPO             0x51
#define ME_SMPTE_OFFSET         0x54
#define ME_TIMESIGNATURE        0x58
#define ME_KEYSIGNATURE         0x59
#define ME_SEQUENCERSPECIFIC    0x7F

#define midime_to_str(type) ((type) == ME_SEQUENCENUMBER ? "ME_SEQUENCENUMBER" : \
                             (type) == ME_TEXTEVENT ? "ME_TEXTEVENT" :	\
                             (type) == ME_COPYRIGHTNOTICE ? "ME_COPYRIGHTNOTICE" : \
                             (type) == ME_NAME ? "ME_NAME" :		\
                             (type) == ME_CUEPOINT ? "ME_CUEPOINT" :	\
                             (type) == ME_ENDOFTRACK ? "ME_ENDOFTRACK" : \
                             (type) == ME_SETTEMPO  ? "ME_SETTEMPO" :   \
                             (type) == ME_SMPTE_OFFSET ? "ME_SMPTE_OFFSET" : \
			     (type) == ME_TIMESIGNATURE ? "ME_TIMESIGNATURE" : \
                             (type) == ME_KEYSIGNATURE  ? "ME_KEYSIGNATURE" : \
			     "UNKNOWN")

typedef struct
{
  byte_t        type;
  uint_t        size;
  byte_t        data[256];
  uint_t        val;
}       midimev_t;

uint_t get_midibuf_deltatime(byte_t *);
uint_t get_varlen_from_idx(byte_t *buffer, uint_t *offset);
uint_t get_varlen_from_ptr(byte_t **buffer);
/* uint_t get_deltatime_from_ptr(byte_t **); /\* a modifier *\/ */

uint_t get_midi_meta_event(midimev_t *meta_ev, byte_t *buffer);
uint_t get_midi_channel_event(midicev_t *chan_ev, byte_t *buffer);

#endif
