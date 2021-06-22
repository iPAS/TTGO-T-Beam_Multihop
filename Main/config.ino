#include "all_headers.h"

#include <Preferences.h>


#define PREF_NAME_SPACE "LORA_RELAY"  // Max 15 chars
#define STR(x) #x

static Preferences pref;

// ----------------------------------------------------------------------------
void config_setup() {
    config_load(R_NODE_ID);
}

// ----------------------------------------------------------------------------
void config_save(pref_reg_t reg) {
    pref.begin(PREF_NAME_SPACE, false);

    switch (reg) {
        case R_NODE_ID:
            pref.putUShort(STR(R_NODE_ID), getAddress());
            break;
    }

    pref.end();
}

// ----------------------------------------------------------------------------
void config_load(pref_reg_t reg) {
    pref.begin(PREF_NAME_SPACE, true);

    switch (reg) {
        case R_NODE_ID:
            setAddress(pref.getUShort(STR(R_NODE_ID), getAddress()));
            break;
    }

    pref.end();
}
