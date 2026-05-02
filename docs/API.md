# API Reference Guide

This document describes the available REST API endpoints for the Checkora chess platform.

---

## 1. Get Game State

**Endpoint:** `/api/state/`  
**Method:** GET  

### Description:
Returns the current game state.

### Response Example:
```json
{
  "board": "current board state",
  "turn": "white",
  "status": "ongoing"
}
```

## 2. Make a Move

**Endpoint:** `/api/move/`  
**Method:** POST  

### Request Body:
```json
{
  "from": "e2",
  "to": "e4"
}
```

### Response Example:
```json
{
  "success": true,
  "message": "Move executed"
}
```

## 3. Get Valid Moves

**Endpoint:** `/api/valid-moves/`  
**Method:** GET  

### Description:
Returns all valid moves for a selected piece.

### Response Example:
```json
{
  "moves": ["e3", "e4"]
}
```

## 4. Start New Game

**Endpoint:** `/api/new-game/`  
**Method:** POST  

### Request Body:
```json
{
  "mode": "ai"
}
```
### Response Example:
```json
{
  "status": "new game started"
}
```
## 5. AI Move

**Endpoint:** `/api/ai-move/`  
**Method:** POST  

### Description:
Triggers AI to make a move.

### Response Example:
```json
{
  "move": "e7e5"
}
```
## 6. Check Promotion

**Endpoint:** `/api/check-promotion/`  
**Method:** GET  

### Description:
Checks if pawn promotion is triggered.

---

## 7. Pause Game

**Endpoint:** `/api/pause/`  
**Method:** POST  

### Description:
Pause or resume the game.

---

## Notes

- All endpoints return JSON responses.
- Ensure proper request formats when using POST methods.
  
