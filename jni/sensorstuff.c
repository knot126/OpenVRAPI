#define USE_GRV 1

typedef struct {
	ASensorEventQueue *queue;
	ovrVector3f orientation;
	uint64_t lastEventTime;
} Gyroscope;

int GyroCallback(int fd, int events, void *this_) {
	Gyroscope *this = this_;
	
	// LOG(ANDROID_LOG_INFO, "ENTER GyroGet()");
	
	while (ASensorEventQueue_hasEvents(this->queue) == 1) {
		ASensorEvent event;
		
		ssize_t event_count = ASensorEventQueue_getEvents(this->queue, &event, 1);
		
		if (event_count != 1) {
			break;
		}
		
#ifdef USE_GRV
		this->orientation.x = event.data[0];
		this->orientation.y = event.data[1];
		this->orientation.z = event.data[2];
#else
		// First event should be used as a reference frame for all others
		if (this->lastEventTime == 0) {
			this->lastEventTime = event.timestamp;
			continue;
		}
		
		// Delta time since last event in seconds
		float delta = ((float) (event.timestamp - this->lastEventTime)) / 1e9;
		
		// Shitty integration :3
		this->orientation.x -= 1.3 * delta * event.data[1];
		this->orientation.y += 1.3 * delta * event.data[0];
		// this->orientation.z += delta * event.data[2];
		
		// Set this as the timestamp for the last event
		this->lastEventTime = event.timestamp;
#endif
	}
	
	return 1;
}

void GyroInit(Gyroscope *this) {
	this->lastEventTime = 0;
	this->orientation = (ovrVector3f) {0.0, 0.0, 0.0};
	
	ALooper *looper = ALooper_forThread();
	
	if (!looper) {
		FATAL("ALooper is NULL! %d", 0);
	}
	
	ASensorManager *mgr = ASensorManager_getInstance();
	
	this->queue = ASensorManager_createEventQueue(mgr, looper, ALOOPER_POLL_CALLBACK, GyroCallback, this);
	
	if (!this->queue) {
		FATAL("this->queue is NULL! %d", 0);
	}
	
#ifdef USE_GRV
	const ASensor *sensor = ASensorManager_getDefaultSensor(mgr, ASENSOR_TYPE_GAME_ROTATION_VECTOR);
#else
	const ASensor *sensor = ASensorManager_getDefaultSensor(mgr, ASENSOR_TYPE_GYROSCOPE);
#endif
	
	if (!sensor) {
		FATAL("Device does not have a gyroscope or failed to get the default one! %d", 0);
	}
	
	int status = ASensorEventQueue_enableSensor(this->queue, sensor);
	
	if (status) {
		// FATAL("ASensorEventQueue_enableSensor() failed! %d", status);
		abort();
	}
	
	ASensorEventQueue_setEventRate(this->queue, sensor, 16666);
}

ovrVector3f GyroGet(Gyroscope *this) {
	return this->orientation;
}

void GyroFree(Gyroscope *this) {
	ASensorManager_destroyEventQueue(ASensorManager_getInstance(), this->queue);
}
