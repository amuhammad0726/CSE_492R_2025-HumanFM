import gi
import requests
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib
from AST_realTime_main.real_time_inference import callback
import numpy as np

# Initializing the Gstreamer object to begin listening for audio.
Gst.init(None)

# Pipeline details in a string.
# Processing UDP packets that have been compressed via OPUS, decompressing,
# and processing in a format that allows Python to read buffered data.
# Modifications made to allow for one channel, 16-bit samples, and 16kHz.
pipeline_str = (
    'udpsrc uri=udp://239.1.1.1:5000 caps="application/x-rtp,media=audio,encoding-name=OPUS,payload=96" '
    '! rtpopusdepay '
    '! opusdec '
    '! audioconvert '
    '! audioresample '
    '! audio/x-raw,rate=16000,format=S16LE,channels=1 '
    '! appsink name=sink emit-signals=true sync=false'
)

# Builds the pipeline and attempt to connect.
pipeline = Gst.parse_launch(pipeline_str)
appsink = pipeline.get_by_name("sink")

# ----- Audio Classification -----
# Set-up a byte array to process binary data (in this case audio).
buffered_audio = bytearray()
# 1 second of mono 16-bit PCM audio @ 16kHz.
'''
16-bit (2 bytes)
Mono (1 channel)
16,000 samples/second (16 kHz)
'''
# 640 bytes ÷ 2 bytes/sample = 320 samples
# 320 samples ÷ 16,000 samples/sec = 0.02 sec = 20 ms
# 16,000 samples/sec * 2 bytes/sample = 32,000 bytes/sec
TARGET_BYTES = 32000

# Pseudo-classifier function that converts raw PCM bytes to NumPy array and
# classifies based on mean power.
def classify_audio(audio_bytes):
    # Convert raw bytes to 16-bit signed integers.
    # Provides us actual wavewform values to work with.
    audio_np = np.frombuffer(audio_bytes, dtype=np.int16)

    # Normalize to float32 [-1, 1] range (optional).
    # Ensures values are easier to compare.
    # 32768.0 is the maximum magnitude of a signed 16-bit number (2¹⁵).
    audio_float = audio_np.astype(np.float32) / 32768.0

    # Compute power (RMS).
    # Square each sample, get the average, and return the square root.
    power = np.sqrt(np.mean(audio_float ** 2))

    # Formatted to five decimal places.
    print(f"📊 Signal Power: {power:.5f}")

    # Pseudo-classification based on power level.
    if power > 0.2:
        print("🔊 Loud sound detected")
        return "LOUD"
    elif power > 0.05:
        print("🟢 Medium sound detected")
        return "MEDIUM"
    else:
        print("🔇 Quiet or background noise")
        return "QUIET"

# ----- Sample Collection -----
# Function that is triggered every time a new audio buffer arrives.
def on_new_sample(sink):
    # Defined as global to ensure global variable can be modified in this function.
    global buffered_audio

    # We pull the most recent sample from app-sink.
    sample = sink.emit("pull-sample")
    # If we indeed pull the sample, we extract the raw buffer data.
    # Mapping into memory so it can be read.
    if sample:
        buffer = sample.get_buffer()
        success, map_info = buffer.map(Gst.MapFlags.READ)
        # If we successfully map, we attempt to process the raw audio data itself.
        if success:
            raw_audio = map_info.data
            buffered_audio += raw_audio

            print(f"📥 Received chunk: {len(raw_audio)} bytes | Buffered: {len(buffered_audio)} bytes")

            if len(buffered_audio) >= TARGET_BYTES:
                print("🧠 1-second buffer ready → Running classification...")
                # output = classify_audio(buffered_audio[:TARGET_BYTES])
                output = callback(buffered_audio[:TARGET_BYTES])
                print(output)
                # send_class_to_node(output)
                buffered_audio = bytearray()  # reset for next second

            # Clean-up after we are done using the current map.
            buffer.unmap(map_info)

    # Returning a success command.
    return Gst.FlowReturn.OK

# ----- Sending Detection to Node Server -----
def send_class_to_node(detected_class):
    url = "http://localhost:8000/HumanFM/classify"
    payload = {"class": detected_class}
    try:
        response = requests.post(url, json=payload)
        if response.status_code == 200:
            print("✅ Successfully sent to Node.js")
        else:
            print(f"❌ Failed to send: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"🚨 Error sending request: {e}")

# Setting-up a callback so the function above is run every time we receive new audio.
appsink.connect("new-sample", on_new_sample)

# Starts the pipeline for listening and processing audio data.
pipeline.set_state(Gst.State.PLAYING)
print("🎤 Python pipeline listening for audio...")

# Creating a while true loop to ensure the pipeline continues to listen for audio
# and doesn't immediately exit.
loop = GLib.MainLoop()
try:
    loop.run()
except KeyboardInterrupt:
    pass

# Shutting down the pipeline and cleaning up.
pipeline.set_state(Gst.State.NULL)