import asyncio
import json
import uvicorn # <-- AJOUTÉ ICI
from contextlib import asynccontextmanager 
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from azure.eventhub.aio import EventHubConsumerClient

# On stocke ici tous les navigateurs connectés
connected_clients = set()

# TA CLÉ AZURE
CONNECTION_STR = ""
CONSUMER_GROUP = ""

# --- 1. FONCTIONS QUI ÉCOUTENT AZURE ---
async def on_event(partition_context, event):
    data_str = event.body_as_str(encoding='UTF-8')
    print(f"Reçu d'Azure : {data_str}")
    
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
    async with client:
        await client.receive(on_event=on_event, starting_position="-1")

# --- GESTION DU DÉMARRAGE ---
@asynccontextmanager
async def lifespan(app: FastAPI):
    task = asyncio.create_task(receive_from_azure())
    yield 
    task.cancel()

app = FastAPI(lifespan=lifespan)

# --- 2. LA ROUTE WEBSOCKET POUR LE SITE WEB ---
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    connected_clients.add(websocket)
    print("Un client Web est connecté !")
    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        connected_clients.remove(websocket)
        print("Client Web déconnecté.")

# --- 3. DÉMARRAGE AUTOMATIQUE DU SERVEUR ---
# <-- CES LIGNES SONT LA MAGIE QUI MANQUAIT !
if __name__ == "__main__":
    print("Démarrage du serveur Web...")
    uvicorn.run(app, host="127.0.0.1", port=8000)