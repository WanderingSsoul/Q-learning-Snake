# 🐍 Snake AI (Q-learning + SFML) & Research

This project is an AI implementation for the Snake game using **Q-learning** and the **SFML** library for visualization and a small **research-style experiment** (how buffer size affects learning speed).

## 🚀 Launch

1. **Download ZIP archive** from releases:  
   [🔗 Download latest version](https://github.com/WanderingSsoul/Q-learning-Snake/releases/latest)
2. **Unpack archive**
3. **Run `Snake_ai.exe`**  
   - The game will start automatically
     
## 🛠 Technologies used
- **C++**
- **SFML** (graphics)
- **Q-learning** (artificial intelligence)
- 
## 🛠working principle:
Uses basic Q-learning which updates values ​​through an experience buffer that stores the past 200,000 moves from the last episodes.
The game of snake displays with SFML library.



## Research summary (mini paper)

### Research question
How does the size of a **sliding buffer of recent states** (`deque_size`) affect how fast a Q-learning agent reaches a **target snake length** `L`?

### Environment
- Grid size: **10×10**
- Episode ends if the snake hits a wall or collides with its own body
- Snake length = number of occupied cells (head + body)
- Food increases the snake length by **1**

### Method
- Tabular **Q-learning** with **ε-greedy** action selection
- We maintain a **recent-state buffer** (sliding window) with capacity `deque_size`
- On each step, the current state is pushed into the buffer; if it overflows, the oldest state is removed (FIFO)
- Updates are performed by sampling states from the current buffer and applying the Q-learning update rule  
  (details + hyperparameters are in the report)

### Metric
For each run we record **T** — the number of episodes until the snake first reaches length **≥ L**.  
The goal is counted in the first episode where the **maximum length reached during that episode** is ≥ L.

### Experimental setup
- We vary two parameters: **target length** `L` and **buffer size** `deque_size`
- All other hyperparameters are fixed
- For each `(L, deque_size)` we run **100** independent runs with different random seeds
- Reported value: **mean number of episodes** to reach the target

### Key findings (high level)
- For small targets (e.g., `L = 10`), the smallest tested buffer performed best
- As the target length increases, the best `deque_size` shifts toward larger values
- Larger buffers help for large targets because they preserve rare late-game states, but they can slow down learning for small targets because the buffer contains too many irrelevant situations.

## 🎥 Demo  
![Game demo](https://github.com/WanderingSsoul/Q-learning-Snake/raw/main/snake.gif)

## 👤 Author  
**Anri Zhan**  
- 📧 Email: [anrizhan555@gmail.com](mailto:anrizhan555@gmail.com)  
- 💼 GitHub: [github.com/WanderingSsoul](https://github.com/Wandering_Ssoul)  
- 🌐 Telegram: **Wandering_Ssoul**
  
