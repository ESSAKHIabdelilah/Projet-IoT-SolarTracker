import asyncio
import json
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from azure.eventhub.aio import EventHubConsumerClient

app = FastAPI()

# On stocke ici tous les navigateurs (React) connectés
connected_clients = set()

# ⚠️ TA CLÉ AZURE (Point de terminaison compatible Event Hub)
CONNECTION_STR = ""
CONSUMER_GROUP = ""

# --- 1. FONCTION QUI ÉCOUTE AZURE ---
async def on_event(partition_context, event):
    # On lit le JSON envoyé par l'Arduino/ESP32
    data_str = event.body_as_str(encoding='UTF-8')
    print(f"Reçu d'Azure : {data_str}")
    
    # On renvoie ce JSON en direct à tous les clients React via WebSocket
    for client in list(connected_clients):
        try:
            await client.send_text(data_str)
        except Exception:
            connected_clients.remove(client)

async def receive_from_azure():
    client = EventHubConsumerClient.from_connection_string(
        conn_str=CONNECTION_STR,
        consumer_group=CONSUMER_GROUP
    )
    # Lancement de l'écoute en tâche de fond (starting_position="-1" pour lire que les nouveaux messages)
    async with client:
        await client.receive(on_event=on_event, starting_position="-1")

# Démarrage de l'écoute Azure quand le serveur Python se lance
@app.on_event("startup")
async def startup_event():
    asyncio.create_task(receive_from_azure())

# --- 2. LA ROUTE WEBSOCKET POUR REACT ---
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    connected_clients.add(websocket)
    print("Un client React est connecté !")
    try:
        while True:
            # On maintient la connexion ouverte
            await websocket.receive_text()
    except WebSocketDisconnect:
        connected_clients.remove(websocket)
        print("Client React déconnecté.")