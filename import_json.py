import json
import os
import speech_recognition as sr  # For voice input
from gtts import gTTS  # For text-to-speech
import pygame  # To play TTS output
from fastapi import FastAPI, HTTPException
from google.cloud import dialogflow_v2 as dialogflow
from pydantic import BaseModel

# Set up Google Cloud Credentials
os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = "/Users/nikunj/Downloads/boreal-mode-454204-s4-7a1bd72eccc6.json"

# Dialogflow API Configuration
DIALOGFLOW_PROJECT_ID = "boreal-mode-454204-s4"
DIALOGFLOW_LANGUAGE_CODE = "en"
SESSION_ID = "esp32_session"

# Initialize FastAPI
app = FastAPI()

# Request Model for Text Commands
class ChatRequest(BaseModel):
    text: str

# Function to communicate with Dialogflow
def detect_intent_texts(text):
    session_client = dialogflow.SessionsClient()
    session = session_client.session_path(DIALOGFLOW_PROJECT_ID, SESSION_ID)

    text_input = dialogflow.TextInput(text=text, language_code=DIALOGFLOW_LANGUAGE_CODE)
    query_input = dialogflow.QueryInput(text=text_input)

    response = session_client.detect_intent(request={"session": session, "query_input": query_input})
    return response.query_result.fulfillment_text

# Function to capture voice input and convert it to text
def get_voice_command():
    recognizer = sr.Recognizer()
    with sr.Microphone() as source:
        print("🎤 Speak Now...")
        recognizer.adjust_for_ambient_noise(source)
        try:
            audio = recognizer.listen(source, timeout=5)
            text = recognizer.recognize_google(audio)
            print(f"🗣️ Recognized Text: {text}")
            return text
        except sr.UnknownValueError:
            return "Sorry, I could not understand."
        except sr.RequestError:
            return "Error connecting to voice recognition service."

# Function to convert text to speech
def text_to_speech(text):
    tts = gTTS(text=text, lang="en")  # Convert text to speech
    tts.save("response.mp3")  # Save speech to a file
    pygame.mixer.init()
    pygame.mixer.music.load("response.mp3")
    pygame.mixer.music.play()

# API Endpoint for Text Input
@app.post("/chat")
async def chat_with_ai(request: ChatRequest):
    response_text = detect_intent_texts(request.text)
    text_to_speech(response_text)  # Speak the response
    return {"response": response_text}

# API Endpoint for Voice Input
@app.get("/voice_chat")
async def chat_with_voice():
    user_speech = get_voice_command()
    if "Sorry" in user_speech or "Error" in user_speech:
        return {"response": user_speech}
    
    response_text = detect_intent_texts(user_speech)
    text_to_speech(response_text)  # Speak the response
    return {"response": response_text}

# Run the server
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

