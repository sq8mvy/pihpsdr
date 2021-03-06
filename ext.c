/* Copyright (C)
* 2017 - John Melton, G0ORX/N6LYT
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <gtk/gtk.h>
#include "discovery.h"
#include "receiver.h"
#include "sliders.h"
#include "toolbar.h"
#include "band_menu.h"
#include "diversity_menu.h"
#include "vfo.h"
#include "radio.h"
#include "radio_menu.h"
#include "new_menu.h"
#include "new_protocol.h"
#ifdef PURESIGNAL
#include "ps_menu.h"
#endif
#include "agc.h"
#include "filter.h"
#include "band.h"
#include "bandstack.h"
#include "noise_menu.h"
#include "wdsp.h"
#include "ext.h"
#include "zoompan.h"

// The following calls functions can be called usig g_idle_add

int ext_vfo_mode_changed(void * data)
{
  int mode=GPOINTER_TO_INT(data);
  vfo_mode_changed(mode);
  return 0;
}

int ext_discovery(void *data) {
  discovery();
  return 0;
}

int ext_set_frequency(void *data) {
  //
  // If new frequency is outside of current band,
  // behave as if the user had chosen the new band
  // via the menu prior to changing the frequency
  //
  long long freq = *(long long *)data;
  int id=active_receiver->id;
  int b = get_band_from_frequency(freq);
  if (b != vfo[id].band) {
    vfo_band_changed(b);
  }
  setFrequency(freq);
  free(data);
  return 0;
}

static guint vfo_timeout=0;

static int vfo_timeout_cb(void * data) {
  vfo_timeout=0;
  vfo_update();
  return 0;
}

//
// ALL calls to vfo_update should go through g_idle_add(ext_vfo_update)
// such that they can be filtered out if they come at high rate
//
int ext_vfo_update(void *data) {
  if (vfo_timeout ==0) {
    vfo_timeout=g_timeout_add(100, vfo_timeout_cb, NULL);
  }
  return 0;
}

int ext_vfo_filter_changed(void *data) {
  vfo_filter_changed(GPOINTER_TO_INT(data));
  return 0;
}

int ext_band_update(void *data) {
  if(data==NULL) {
    start_band();
  } else {
    band_select_cb(NULL,data);
  }
  return 0;
}

int ext_bandstack_update(void *data) {
  start_bandstack();
  return 0;
}

int ext_mode_update(void *data) {
  start_mode();
  return 0;
}

int ext_filter_update(void *data) {
  start_filter();
  return 0;
}

int ext_frequency_update(void *data) {
  start_vfo();
  return 0;
}

int ext_memory_update(void *data) {
  start_store();
  return 0;
}

int ext_noise_update(void *data) {
  start_noise();
  return 0;
}

int ext_mox_update(void *data) {
  mox_update(GPOINTER_TO_INT(data));
  return 0;
}

int ext_tune_update(void *data) {
  tune_update(GPOINTER_TO_INT(data));
  return 0;
}

int ext_vox_changed(void *data) {
  vox_changed(GPOINTER_TO_INT(data));
  return 0;
}

int ext_update_agc_gain(void *data) {
  update_agc_gain(GPOINTER_TO_INT(data));
  free(data);
  return 0;
}

int ext_update_af_gain(void *data) {
  update_af_gain();
  return 0;
}

int ext_calc_drive_level(void *data) {
  calcDriveLevel();
  return 0;
}

int ext_vfo_band_changed(void *data) {
  vfo_band_changed(GPOINTER_TO_INT(data));
  return 0;
}

int ext_radio_change_sample_rate(void *data) {
  radio_change_sample_rate(GPOINTER_TO_INT(data));
  return 0;
}

int ext_update_squelch(void *data) {
  set_squelch();
  return 0;
}

int ext_sliders_update(void *data) {
  sliders_update();
  return 0;
}

#ifdef PURESIGNAL
int ext_tx_set_ps(void *data) {
  if(can_transmit) {
    int state=GPOINTER_TO_INT(data);
    tx_set_ps(transmitter, state);
  }
  return 0;
}
#endif

int ext_update_vfo_step(void *data) {
  int direction=GPOINTER_TO_INT(data);
  int i=0;
  while(steps[i]!=step && steps[i]!=0) {
    i++;
  }

  if(steps[i]!=0) {
    if(direction>0) {
      i++;
      if(steps[i]!=0) {
        step=steps[i];
      }
    } else {
      i--;
      if(i>=0) {
        step=steps[i];
      }
    }
  }
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_vfo_step(void *data) {
  int step=GPOINTER_TO_INT(data);
  vfo_step(step);
  return 0;
}

int ext_vfo_id_step(void *data) {
  int *ip=(int *) data;
  int id=ip[0];
  int step=ip[1];
  vfo_id_step(id,step);
  free(data);
  return 0;
}

int ext_set_mic_gain(void * data) {
  double d=*(double *)data;
  set_mic_gain(d);
  free(data);
  return 0;
}

int ext_set_agc_gain(void *data) {
  double d=*(double *)data;
  set_agc_gain(active_receiver->id,d);
  free(data);
  return 0;
}
 
int ext_set_drive(void *data) {
  double d=*(double *)data;
  set_drive(d);
  free(data);
  return 0;
}

int ext_set_compression(void *data) {
  if(can_transmit) {
    set_compression(transmitter);
  }
  return 0;
}

int ext_vfo_a_swap_b(void *data) {
  vfo_a_swap_b();
  return 0;
}

int ext_vfo_a_to_b(void *data) {
  vfo_a_to_b();
  return 0;
}

int ext_vfo_b_to_a(void *data) {
  vfo_b_to_a();
  return 0;
}

int ext_update_att_preamp(void *data) {
  update_att_preamp();
  return 0;
}

int ext_set_alex_attenuation(void *data) {
  int val=GPOINTER_TO_INT(data);
  set_alex_attenuation(val);
  return 0;
}

int ext_set_attenuation_value(void *data) {
  double d=*(double *)data;
  set_attenuation_value(d);
  free(data);
  return 0;
}


#ifdef PURESIGNAL
int ext_ps_update(void *data) {
  if(can_transmit) {
    if(transmitter->puresignal==0) {
      tx_set_ps(transmitter,1);
    } else {
      tx_set_ps(transmitter,0);
    }
  }
  return 0;
}
#endif

int ext_two_tone(void *data) {
  if(can_transmit) {
    int state=transmitter->twotone?0:1;
    tx_set_twotone(transmitter,state);
  }
  return 0;
}

int ext_nr_update(void *data) {
  if(active_receiver->nr==0 && active_receiver->nr2==0) {
    active_receiver->nr=1;
    active_receiver->nr2=0;
    mode_settings[vfo[active_receiver->id].mode].nr=1;
    mode_settings[vfo[active_receiver->id].mode].nr2=0;
  } else if(active_receiver->nr==1 && active_receiver->nr2==0) {
    active_receiver->nr=0;
    active_receiver->nr2=1;
    mode_settings[vfo[active_receiver->id].mode].nr=0;
    mode_settings[vfo[active_receiver->id].mode].nr2=1;
  } else if(active_receiver->nr==0 && active_receiver->nr2==1) {
    active_receiver->nr=0;
    active_receiver->nr2=0;
    mode_settings[vfo[active_receiver->id].mode].nr=0;
    mode_settings[vfo[active_receiver->id].mode].nr2=0;
  }
  SetRXAANRRun(active_receiver->id, active_receiver->nr);
  SetRXAEMNRRun(active_receiver->id, active_receiver->nr2);
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_nb_update(void *data) {
  if(active_receiver->nb==0 && active_receiver->nb2==0) {
    active_receiver->nb=1;
    active_receiver->nb2=0;
    mode_settings[vfo[active_receiver->id].mode].nb=1;
    mode_settings[vfo[active_receiver->id].mode].nb2=0;
  } else if(active_receiver->nb==1 && active_receiver->nb2==0) {
    active_receiver->nb=0;
    active_receiver->nb2=1;
    mode_settings[vfo[active_receiver->id].mode].nb=0;
    mode_settings[vfo[active_receiver->id].mode].nb2=1;
  } else if(active_receiver->nb==0 && active_receiver->nb2==1) {
    active_receiver->nb=0;
    active_receiver->nb2=0;
    mode_settings[vfo[active_receiver->id].mode].nb=0;
    mode_settings[vfo[active_receiver->id].mode].nb2=0;
  }
  SetEXTANBRun(active_receiver->id, active_receiver->nb);
  SetEXTNOBRun(active_receiver->id, active_receiver->nb2);
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_snb_update(void *data) {
  if(active_receiver->snb==0) {
    active_receiver->snb=1;
    mode_settings[vfo[active_receiver->id].mode].snb=1;
  } else {
    active_receiver->snb=0;
    mode_settings[vfo[active_receiver->id].mode].snb=0;
  }
  SetRXASNBARun(active_receiver->id, active_receiver->snb);
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_band_plus(void *data) {
  long long frequency_min=radio->frequency_min;
  long long frequency_max=radio->frequency_max;
  int id=active_receiver->id;
  int b=vfo[id].band;
  BAND *band;
  int found=0;
  while(!found) {
    b++;
    if(b>=BANDS+XVTRS) b=0;
    band=(BAND*)band_get_band(b);
    if(strlen(band->title)>0) {
      if(b<BANDS) {
        if(!(band->frequencyMin==0.0 && band->frequencyMax==0.0)) {
          if(band->frequencyMin<frequency_min || band->frequencyMax>frequency_max) {
            continue;
          }
        }
      }
      vfo_band_changed(b);
      found=1;
    }
  }
  return 0;
}


int ext_band_minus(void *data) {
  long long frequency_min=radio->frequency_min;
  long long frequency_max=radio->frequency_max;
  int id=active_receiver->id;
  int b=vfo[id].band;
  BAND *band;
  int found=0;
  while(!found) {
    b--;
    if(b<0) b=BANDS+XVTRS-1;
    band=(BAND*)band_get_band(b);
    if(strlen(band->title)>0) {
      if(b<BANDS) {
        if(band->frequencyMin<frequency_min || band->frequencyMax>frequency_max) {
          continue;
        }
      }
      vfo_band_changed(b);
      found=1;
    }
  }
  return 0;
}

int ext_bandstack_plus(void *data) {
  BAND *band=band_get_band(vfo[active_receiver->id].band);
  BANDSTACK *bandstack=band->bandstack;
  int b=vfo[active_receiver->id].bandstack+1;
  if(b>=bandstack->entries) b=0;
  vfo_bandstack_changed(b);
  return 0;
}

int ext_bandstack_minus(void *data) {
  BAND *band=band_get_band(vfo[active_receiver->id].band);
  BANDSTACK *bandstack=band->bandstack;
  int b=vfo[active_receiver->id].bandstack-1;
  if(b<0) b=bandstack->entries-1;;
  vfo_bandstack_changed(b);
  return 0;
}

int ext_lock_update(void *data) {
  locked=locked==1?0:1;
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_rit_update(void *data) {
  vfo[active_receiver->id].rit_enabled=vfo[active_receiver->id].rit_enabled==1?0:1;
  receiver_frequency_changed(active_receiver);
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_rit_clear(void *data) {
  vfo[active_receiver->id].rit=0;
  receiver_frequency_changed(active_receiver);
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_xit_update(void *data) {
  if(can_transmit) {
    transmitter->xit_enabled=transmitter->xit_enabled==1?0:1;
    if(protocol==NEW_PROTOCOL) {
      schedule_high_priority();
    }
  }
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_xit_clear(void *data) {
  if(can_transmit) {
    transmitter->xit=0;
    g_idle_add(ext_vfo_update, NULL);
  }
  return 0;
}

int ext_filter_plus(void *data) {
  int f=vfo[active_receiver->id].filter-1;
  if(f<0) f=FILTERS-1;
  vfo_filter_changed(f);
  return 0;
}

int ext_filter_minus(void *data) {
  int f=vfo[active_receiver->id].filter+1;
  if(f>=FILTERS) f=0;
  vfo_filter_changed(f);
  return 0;
}

int ext_mode_plus(void *data) {
  int mode=vfo[active_receiver->id].mode;
  mode++;
  if(mode>=MODES) mode=0;
  vfo_mode_changed(mode);
  return 0;
}

int ext_mode_minus(void *data) {
  int mode=vfo[active_receiver->id].mode;
  mode--;
  if(mode<0) mode=MODES-1;
  vfo_mode_changed(mode);
  return 0;
}

int ext_ctun_update(void *data) {
  int id=active_receiver->id;
  vfo[id].ctun=vfo[id].ctun==1?0:1;
  if(!vfo[id].ctun) {
    vfo[id].offset=0;
  }
  vfo[id].ctun_frequency=vfo[id].frequency;
  set_offset(active_receiver,vfo[id].offset);
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_agc_update(void *data) {
  active_receiver->agc++;
  if(active_receiver->agc>+AGC_LAST) {
    active_receiver->agc=0;
  }
  set_agc(active_receiver, active_receiver->agc);
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_split_toggle(void *data) {
  if(can_transmit) {
    split=split==1?0:1;
    tx_set_mode(transmitter,get_tx_mode());
    g_idle_add(ext_vfo_update, NULL);
  }
  return 0;
}

int ext_start_rx(void *data) {
  start_rx();
  return 0;
}

int ext_start_tx(void *data) {
  start_tx();
  return 0;
}

int ext_diversity_update(void *data) {
  int menu=GPOINTER_TO_INT(data);
  if(menu) {
    start_diversity();
  } else {
    diversity_enabled=diversity_enabled==1?0:1;
    if (protocol == NEW_PROTOCOL) {
      schedule_high_priority();
      schedule_receive_specific();
    }
    g_idle_add(ext_vfo_update, NULL);
  }
  return 0;
}

int ext_diversity_change_gain(void *data) {
  double *dp = (double *) data;
  update_diversity_gain(*dp);
  free(dp);
  return 0;
}

int ext_diversity_change_phase(void *data) {
  double *dp = (double *) data;
  update_diversity_phase(*dp);
  free(dp);
  return 0;
}

#ifdef PURESIGNAL
int ext_start_ps(void *data) {
  start_ps();
  return 0;
}
#endif

int ext_sat_update(void *data) {
  switch(sat_mode) {
    case SAT_NONE:
      sat_mode=SAT_MODE;
      break;
    case SAT_MODE:
      sat_mode=RSAT_MODE;
      break;
    case RSAT_MODE:
      sat_mode=SAT_NONE;
      break;
  }
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_function_update(void *data) {
  function++;
  if(function>MAX_FUNCTION) {
    function=0;
  }
  update_toolbar_labels();
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_set_rf_gain(void *data) {
  int pos=GPOINTER_TO_INT(data);
  double value;
  value=(double)pos;
  if(value<-12.0) {
    value=-12.0;
  } else if(value>48.0) {
    value=48.0;
  }
  set_rf_gain(active_receiver->id,value);
  return 0;
}

int ext_update_noise(void *data) {
  update_noise();
  return 0;
}

int ext_set_duplex(void *data) {
  setDuplex();
  return 0;
}

int ext_remote_command(void *data) {
  REMOTE_COMMAND *p=(REMOTE_COMMAND *)data;
  switch(p->cmd) {
    case RX_FREQ_CMD:
      {
        int b = get_band_from_frequency(p->data.frequency);
        if (b != vfo[p->id].band) {
          vfo_band_changed(b);
        }
        setFrequency(p->data.frequency);
      }
      break;
    case RX_MOVE_CMD:
      vfo_move(p->data.frequency,TRUE);
      break;
    case RX_MOVETO_CMD:
      vfo_move_to(p->data.frequency);
      break;
    case RX_MODE_CMD:
      vfo_mode_changed(p->data.mode);
      break;
    case RX_FILTER_CMD:
      vfo_filter_changed(p->data.filter);
      break;
    case RX_AGC_CMD:
      active_receiver->agc=p->data.agc;
      set_agc(active_receiver, active_receiver->agc);
      vfo_update();
      break;
    case RX_NR_CMD:
      break;
    case RX_NB_CMD:
      break;
    case RX_SNB_CMD:
      break;
    case RX_SPLIT_CMD:
      break;
    case RX_SAT_CMD:
      break;
    case RX_DUP_CMD:
      break;
  }
  g_free(data);
  return 0;
}

int ext_anf_update(void *data) {
  if(active_receiver->anf==0) {
    active_receiver->anf=1;
    mode_settings[vfo[active_receiver->id].mode].anf=1;
  } else {
    active_receiver->snb=0;
    mode_settings[vfo[active_receiver->id].mode].anf=0;
  }
  SetRXAANFRun(active_receiver->id, active_receiver->anf);
  g_idle_add(ext_vfo_update, NULL);
  return 0;
}

int ext_mute_update(void *data) {
  active_receiver->mute_radio=!active_receiver->mute_radio;
  return 0;
}

int ext_zoom_update(void *data) {
  update_zoom((double)GPOINTER_TO_INT(data));
  return 0;
}

int ext_zoom_set(void *data) {
  int pos=GPOINTER_TO_INT(data);
  double zoom=((double)pos/(100.0/7.0))+1.0;
  if((int)zoom!=active_receiver->zoom) {
    set_zoom(active_receiver->id,(double)zoom);
  }
  return 0;
}

int ext_pan_update(void *data) {
  update_pan((double)GPOINTER_TO_INT(data));
  return 0;
}

int ext_pan_set(void *data) {
  if(active_receiver->zoom>1) {
    int pos=GPOINTER_TO_INT(data);
    double pan=(double)((active_receiver->zoom-1)*active_receiver->width)*((double)pos/100.0);
    set_pan(active_receiver->id,(double)pan);
  }
  return 0;
}
