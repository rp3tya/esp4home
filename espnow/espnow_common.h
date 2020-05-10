uint8_t mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct struct_status {
  int slave_id;
  int buttons;
  int switches;
  float sensors[25];
} struct_status;

typedef struct struct_command {
  int slave_id;
  int switch_id;
  int state;
} struct_command;
