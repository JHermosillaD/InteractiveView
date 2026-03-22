# Hand-Controlled 3D Camera

![OF](https://img.shields.io/badge/openFrameworks-0.12.1-black?style=flat-square)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square&logo=c%2B%2B)
![MediaPipe](https://img.shields.io/badge/MediaPipe-Tracking-blueviolet?style=flat-square)
![OS](https://img.shields.io/badge/OS-Linux-orange?style=flat-square&logo=linux&logoColor=black)
![License](https://img.shields.io/badge/License-MIT-red?style=flat-square)

A real-time interactive 3D camera controller demo built with openFrameworks and MediaPipe.

### Project Structure
```text
├── src/
│   ├── main.cpp
│   ├── ofApp.h
│   ├── ofApp.cpp
│   ├── HandCameraController.h
│   ├── HandCameraController.cpp
│   ├── LandmarkSmoother.h
│   ├── LandmarkSmoother.cpp
│   └── V4L2CameraSettings.h
```

### Controls & Gestures

The user manipulates a virtual 3D camera (Pan, Tilt, and Zoom) through hand gestures. The camera is driven by a state machine (`IDLE`, `LOCKED`, `TWO_HANDS`). To engage the camera, form a claw with both hands (with at least 3 non-thumb fingers).

| Input / Gesture | Action |
|---|---|
| **Left Wrist (Claw)** | Controls Pan (Yaw / Y-Axis) |
| **Right Wrist (Claw)** | Controls Tilt (Pitch / X-Axis) |
| **Hand Spread** | Controls Zoom (spread = zoom in, pinch = zoom out) |
| `R` / `r` | Reset camera to default position (0,0,0) and distance (750) |
| `F` / `f` | Toggle fullscreen |

Releasing the claw gesture initiates a `LOCKED` state to prevent jitter before returning to `IDLE`.

### Theoretical Background

To reduce jitter the project implements a **3D One Euro Filter**. The smoothing factor $\alpha$ is calculated per-frame from the time delta $\Delta t$ and cutoff frequency $f_c$:
$$\tau = \frac{1}{2\pi f_c}, \qquad \alpha = \frac{1}{1 + \frac{\tau}{\Delta t}}$$

The filtered position is updated as:
$$\mathbf{X}_i = \alpha \, \mathbf{x}_i + (1 - \alpha)\, \mathbf{X}_{i-1}$$

The adaptive cutoff uses a filtered estimate of the velocity magnitude. Raw velocity is first smoothed with a fixed derivative cutoff $f_d$:
$$\dot{\mathbf{X}}_i = \alpha_d \cdot \frac{\mathbf{x}_i - \mathbf{X}_{i-1}}{\Delta t} + (1 - \alpha_d)\cdot \dot{\mathbf{X}}_{i-1}$$

The cutoff then scales with the 3D speed, controlled by $\beta$:

$$f_c = f_{min} + \beta \cdot \|\dot{\mathbf{X}}_i\|$$

Then, the raw hand joint coordinates are translated into smooth camera movements using basis vector construction and quaternion decomposition through the following steps:

  **1. Basis Vector Construction**

An orthonormal basis is built from: Wrist $\mathbf{w}$, Index MCP $\mathbf{p}_R$, Middle MCP $\mathbf{p}_{mid}$, and Pinky MCP $\mathbf{p}_L$ landmarks.

The palm normal is estimated as:

$$\mathbf{n} = \pm\, \frac{(\mathbf{p}_L - \mathbf{p}_R) \times (\mathbf{p}_R - \mathbf{w})}{\|(\mathbf{p}_L - \mathbf{p}_R) \times (\mathbf{p}_R - \mathbf{w})\|}$$

The sign is flipped for right hands to ensure normal consistently. A up direction is defined along the middle finger, then the basis is orthonormalized via Gram-Schmidt equations:

$$\mathbf{u}_{raw} = \frac{\mathbf{p}_{mid} - \mathbf{w}}{\|\mathbf{p}_{mid} - \mathbf{w}\|}, \qquad \mathbf{s} = \frac{\mathbf{u}_{raw} \times \mathbf{n}}{\|\mathbf{u}_{raw} \times \mathbf{n}\|}, \qquad \mathbf{u} = \mathbf{n} \times \mathbf{s}$$

The result is cast to a quaternion representing the hand's absolute orientation.

  **2. Swing-Twist Decomposition**

To extract pan and tilt, the delta quaternion $q = (q_w, \mathbf{r})$ between the current and reference hand orientation is decomposed around a target axis $\mathbf{a}$:

$$\mathbf{p} = (\mathbf{r} \cdot \mathbf{a})\,\mathbf{a}, \qquad q_{twist} = \frac{(q_w,\;\mathbf{p})}{\|(q_w,\;\mathbf{p})\|}$$

  **3. Applying View Logic**

- **Pan:** Left hand delta decomposed around the world Y-axis → $q_{yaw}$
- **Tilt:** Right hand delta decomposed around the camera's local X-axis → $q_{pitch}$
- **Zoom:** Linear ratio of reference spread $S_{ref}$ to current spread $S_{cur}$:

$$D = \mathrm{clamp}\!\left(D_{ref} \times \frac{S_{ref}}{S_{cur}},\; D_{min},\; D_{max}\right)$$

  **4. Smoothing**

Each frame is smoothed using the $s$ as a retention factor:

$$t = 1 - s, \qquad \mathbf{q}_{smooth} = \mathrm{slerp}(\mathbf{q}_{smooth},\; \mathbf{q}_{cam},\; t), \qquad D_{smooth} \mathrel{+}= t\,(D - D_{smooth})$$

---

### Dependencies

- The project applies a custom camera profile via the `v4l2` API.
- The HandTracker implementation is the **[ofxMediaPipe](https://github.com/design-io/ofxMediaPipePython)** Add-on developed by @design-io.
