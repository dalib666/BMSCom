

extern void Mqtt_init();

/* call in 60 sec*/
extern void Mqtt_main();
extern void Mqtt_loopQ(void *);
extern void Mqtt_loopS(void *);
bool MQTT_Check();