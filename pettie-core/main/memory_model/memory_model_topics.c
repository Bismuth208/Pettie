#include "memory_model_topics.h"

#include <esp_attr.h>

// ----------------------------------------------------------------------
// clang-format off



// -----------
topic_t ServoCalD0Topic =
{
  .data = {
    "Pettie/Cal/D0",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

topic_t ServoCalD1Topic =
{
  .data = {
    "Pettie/Cal/D1",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

topic_t ServoCalD2Topic =
{
  .data = {
    "Pettie/Cal/D2",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

#if 0
topic_t ServoCalD3Topic =
{
  .data = {
    "Pettie/Cal/D3",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};
#endif

topic_t ServoCalD4Topic =
{
  .data = {
    "Pettie/Cal/D4",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

topic_t ServoCalD5Topic =
{
  .data = {
    "Pettie/Cal/D5",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

topic_t ServoCalD6Topic =
{
  .data = {
    "Pettie/Cal/D6",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

topic_t ServoCalD7Topic =
{
  .data = {
    "Pettie/Cal/D7",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

topic_t ServoCalD8Topic =
{
  .data = {
    "Pettie/Cal/D8",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

topic_t ServoCalUpdateTopic =
{
  .data = {
    "Pettie/Cal/Update",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = true, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};

topic_t ServoCalSaveTopic =
{
  .data = {
    "Pettie/Cal/Save",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = true, .need_sub = true,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};


// -----------
#if 0
topic_t DistanceTopic =
{
  .data = {
    "Pettie/Sonar",
    "0",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = true, .need_sub = false,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};
#endif

// -----------
topic_t WiFiRSSiTopic =
{
  .data = {
    "Pettie/RSSi",
    "-?? dBm",
    .text_len = 0,
    .value_len = 0,
    .hash = 0
  },
  .status = {
    .do_publish = false, .need_publish = false, .need_sub = false,
    // 
    .update_timestamp = 0,
    .pub_timeout = 0,
    .pub_timestamp = 0
  }
};


// clang-format on

// ----------------------------------------------------------------------
topic_db_t topics_db[] DRAM_ATTR = {

  // ----------------
    { TOPIC_ID_SERVO_CAL_D0,                    &ServoCalD0Topic,                false},
    { TOPIC_ID_SERVO_CAL_D1,                    &ServoCalD1Topic,                false},
    { TOPIC_ID_SERVO_CAL_D2,                    &ServoCalD2Topic,                false},
    // { TOPIC_ID_SERVO_CAL_D3,                    &ServoCalD3Topic,                false},
    { TOPIC_ID_SERVO_CAL_D4,                    &ServoCalD4Topic,                false},
    { TOPIC_ID_SERVO_CAL_D5,                    &ServoCalD5Topic,                false},
    { TOPIC_ID_SERVO_CAL_D6,                    &ServoCalD6Topic,                false},
    { TOPIC_ID_SERVO_CAL_D7,                    &ServoCalD7Topic,                false},
    { TOPIC_ID_SERVO_CAL_D8,                    &ServoCalD8Topic,                false},
    { TOPIC_ID_SERVO_CAL_UPDATE,                &ServoCalUpdateTopic,              false},
    { TOPIC_ID_SERVO_CAL_SAVE,                  &ServoCalSaveTopic,              false},
    
  // ----------------
    // { TOPIC_ID_DISTANCE,                    &DistanceTopic,                false},
  
  // ----------------
    { TOPIC_ID_RSSI_VAL,                    &WiFiRSSiTopic,                false},

  // ----------------
    { TOPIC_ID_MAX,                         NULL,                          false}
};

// NULL is also counted :)
_Static_assert((sizeof(topics_db) / sizeof(topics_db[0])) == (TOPIC_ID_MAX + 1), "TOPIC_ID size fail!");
