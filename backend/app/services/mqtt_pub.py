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

    def wait_for_connect(self, timeout_secs: float = 10.0) -> bool:
        """Blocks until MQTT is connected or timeout expires. Returns True if connected."""
        deadline = time.time() + timeout_secs
        while not self.connected and time.time() < deadline:
            time.sleep(0.2)
        if not self.connected:
            logger.warning(f"MQTT not connected after {timeout_secs}s wait.")
        return self.connected

    def publish(self, topic: str, payload, retain: bool = True) -> bool:
        """Publishes a JSON payload to a specified topic.
        
        Accepts dict or list as payload. Skips publish if not connected.
        Returns True if message was queued/published successfully.
        """
        if not self.connected:
            logger.warning(f"MQTT not connected — skipping publish to {topic}.")
            return False
        try:
            payload_str = json.dumps(payload, ensure_ascii=False)
            info = self.client.publish(topic, payload_str, qos=1, retain=retain)
            # Wait up to 5 seconds for the broker to acknowledge
            info.wait_for_publish(timeout=5.0)
            if info.is_published():
                logger.info(f"✅ Published to {topic} ({len(payload_str)} bytes)")
            else:
                logger.warning(f"⚠️ Publish to {topic} did not confirm within timeout (rc={info.rc})")
            return info.is_published()
        except Exception as e:
            logger.error(f"Failed to publish to {topic}: {e}")
            return False

    def disconnect(self):
        self.client.loop_stop()
        self.client.disconnect()

# Global Singleton instance
mqtt_service = MqttService()
