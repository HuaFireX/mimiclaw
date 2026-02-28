#!/usr/bin/env python3
import asyncio
import websockets
import json

IP = "192.168.37.166"
PORT = 18789

async def main():
    uri = f"ws://{IP}:{PORT}"
    async with websockets.connect(uri) as ws:
        print(f"Connected (ws://{IP}:{PORT})")
        print("Type quit to exit\n")
        
        # 启动接收任务，使收发并发进行，避免 input() 阻塞接收
        async def receiver(ws):
            try:
                while True:
                    r = await ws.recv()
                    d = json.loads(r)
                    content = d.get("content", "")
                    
                    if "mimi is working" in content or "🐱" in content:
                        continue
                    
                    print(f"\rMimiClaw: {content}\nYou: ", end="", flush=True)
            except websockets.ConnectionClosed:
                pass

        receive_task = asyncio.create_task(receiver(ws))

        while True:
            # 使用 run_in_executor 避免 input() 阻塞整个事件循环
            m = await asyncio.get_event_loop().run_in_executor(None, lambda: input("You: "))
            if m.lower() == "quit":
                break
            
            await ws.send(json.dumps({"type": "message", "content": m, "chat_id": "py"}))

        receive_task.cancel()

asyncio.run(main())
