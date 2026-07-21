import json
import logging
import time
import paho.mqtt.client as mqtt
from app.config import settings

logger = logging.getLogger("mqtt_service")
logging.basicConfig(level=logging.INFO)

class MqttService:
    def __init__(self):
        self.client = mqtt.Client(client_id="auradeck_backend_pub")
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.connected = False
        
    def _on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            logger.info("Successfully connected to MQTT Broker!")
            self.connected = True
        else:
            logger.error(f"Failed to connect to MQTT Broker, return code {rc}")
            self.connected = False

    def _on_disconnect(self, client, userdata, rc):
        logger.warning(f"Disconnected from MQTT Broker (rc={rc}). Reconnecting in background...")
        self.connected = False

    def connect(self):
        """Asynchronously connect to the MQTT Broker with retries."""
        retries = 5
        while retries > 0:
            try:
                logger.info(f"Connecting to MQTT Broker at {settings.MQTT_HOST}:{settings.MQTT_PORT}...")
                self.client.connect_async(settings.MQTT_HOST, settings.MQTT_PORT, keepalive=60)
                self.client.loop_start()
                return
            except Exception as e:
                retries -= 1
                logger.warning(f"MQTT Broker connection attempt failed: {e}. Retries remaining: {retries}")
                time.sleep(3)
        logger.critical("Could not establish initial connection to MQTT Broker. Will continue attempting to reconnect in background.")
        # Start client loop anyway, paho will automatically attempt reconnects
        self.client.loop_start()

    def publish(self, topic: str, payload: dict) -> bool:
        """Publishes a JSON payload to a specified topic."""
        try:
            payload_str = json.dumps(payload, ensure_ascii=False)
            info = self.client.publish(topic, payload_str, qos=1, retain=True)
            info.wait_for_publish()
            logger.info(f"Published to {topic}: {payload_str}")
            return info.is_published()
        except Exception as e:
            logger.error(f"Failed to publish to {topic}: {e}")
            return False

    def disconnect(self):
        self.client.loop_stop()
        self.client.disconnect()

# Global Singleton instance
mqtt_service = MqttService()
