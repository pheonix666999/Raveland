/* Demo-only GUI state (no audio engine). */
const LAYER_STACKS = [
  "Hard Dance Stack 01",
  "Hard Dance Stack 02",
  "Rave Techno Stack 01",
  "Trance Stack 01",
  "Virus Lead Stack (mock)",
  "JP Lead Stack (mock)",
];

const DEFAULT_STATE = {
  layers: {
    A: { enabled: true, stack: LAYER_STACKS[0], tune: 0, startRand: 35, attack: 10, release: 220 },
    B: { enabled: true, stack: LAYER_STACKS[2], tune: 0, startRand: 45, attack: 8, release: 240 },
    C: { enabled: false, stack: LAYER_STACKS[3], tune: -12, startRand: 55, attack: 12, release: 300 },
  },
  osc: {
    1: { enabled: true, wave: "saw", voices: 16, detune: 55, semi: 0, fine: 0, level: 85 },
    2: { enabled: true, wave: "saw2", voices: 16, detune: 62, semi: 0, fine: -7, level: 75 },
    3: { enabled: true, wave: "pulse", voices: 12, detune: 38, semi: 12, fine: 3, level: 62 },
    4: { enabled: false, wave: "sine", voices: 1, detune: 0, semi: -12, fine: 0, level: 35 },
  },
  mod: { mode: "lfo", rate: 2.0, amount: 45, target: "filter.cutoff", shape: "sine" },
  fx: {
    chain: ["filter", "chorus", "delay", "reverb", "dist"],
    filter: { enabled: true, type: "lp", cutoff: 9800, reso: 18 },
    chorus: { enabled: true, rate: 0.35, mix: 35 },
    delay: { enabled: true, time: 260, fb: 26, mix: 18 },
    reverb: { enabled: true, mode: "hall", size: 55, damp: 28, mix: 22 },
    dist: { enabled: true, type: "soft", drive: 32, tone: 8, mix: 26 },
  },
  mono: { enabled: false, legato: false, portamento: 0, curve: 0 },
  master: { volume: 78 },
};

const PRESETS = [
  {
    name: "INIT — Clean Saw Lead",
    state: DEFAULT_STATE,
  },
  {
    name: "Rave — Wide SuperSaw Stack",
    state: {
      ...DEFAULT_STATE,
      layers: {
        A: { ...DEFAULT_STATE.layers.A, enabled: true, stack: LAYER_STACKS[0], startRand: 65 },
        B: { ...DEFAULT_STATE.layers.B, enabled: true, stack: LAYER_STACKS[1], startRand: 70 },
        C: { ...DEFAULT_STATE.layers.C, enabled: false },
      },
      osc: {
        ...DEFAULT_STATE.osc,
        1: { ...DEFAULT_STATE.osc[1], wave: "saw", voices: 24, detune: 72, level: 88 },
        2: { ...DEFAULT_STATE.osc[2], wave: "saw2", voices: 24, detune: 78, level: 78 },
        3: { ...DEFAULT_STATE.osc[3], wave: "saw3", voices: 16, detune: 60, semi: 0, level: 62 },
      },
      fx: {
        ...DEFAULT_STATE.fx,
        chorus: { ...DEFAULT_STATE.fx.chorus, mix: 55, rate: 0.28 },
        delay: { ...DEFAULT_STATE.fx.delay, mix: 26, time: 320, fb: 34 },
        reverb: { ...DEFAULT_STATE.fx.reverb, mix: 28, size: 65, mode: "hall" },
        dist: { ...DEFAULT_STATE.fx.dist, drive: 40, mix: 20, type: "tape" },
      },
      mod: { ...DEFAULT_STATE.mod, mode: "lfo", rate: 1.5, amount: 55, target: "osc1.detune", shape: "triangle" },
    },
  },
  {
    name: "Trance — Tight JP-ish Pluck",
    state: {
      ...DEFAULT_STATE,
      layers: {
        A: { ...DEFAULT_STATE.layers.A, enabled: true, stack: LAYER_STACKS[5], attack: 0, release: 140, startRand: 35 },
        B: { ...DEFAULT_STATE.layers.B, enabled: false },
        C: { ...DEFAULT_STATE.layers.C, enabled: false },
      },
      osc: {
        ...DEFAULT_STATE.osc,
        1: { ...DEFAULT_STATE.osc[1], wave: "jp", voices: 12, detune: 40, level: 80 },
        2: { ...DEFAULT_STATE.osc[2], enabled: false },
        3: { ...DEFAULT_STATE.osc[3], wave: "pulse", voices: 8, detune: 18, semi: 0, level: 55 },
      },
      fx: {
        ...DEFAULT_STATE.fx,
        filter: { ...DEFAULT_STATE.fx.filter, cutoff: 4800, reso: 24, type: "lp" },
        chorus: { ...DEFAULT_STATE.fx.chorus, mix: 25, rate: 0.45 },
        delay: { ...DEFAULT_STATE.fx.delay, mix: 18, time: 210, fb: 22 },
        reverb: { ...DEFAULT_STATE.fx.reverb, mode: "room", mix: 14, size: 30 },
        dist: { ...DEFAULT_STATE.fx.dist, type: "hard", drive: 18, mix: 12, tone: 15 },
      },
      mono: { enabled: true, legato: true, portamento: 55, curve: -20 },
      mod: { ...DEFAULT_STATE.mod, mode: "env", rate: 0.6, amount: 65, target: "filter.cutoff", shape: "exp" },
    },
  },
];

let state = structuredClone(DEFAULT_STATE);
let currentPresetIndex = 0;
let scopeState = null;

function hashString(str) {
  // FNV-1a 32-bit
  let h = 2166136261;
  for (let i = 0; i < str.length; i++) {
    h ^= str.charCodeAt(i);
    h = Math.imul(h, 16777619);
  }
  return h >>> 0;
}

function mulberry32(seed) {
  let a = seed >>> 0;
  return function () {
    a |= 0;
    a = (a + 0x6d2b79f5) | 0;
    let t = Math.imul(a ^ (a >>> 15), 1 | a);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

function initBackground() {
  const canvas = document.getElementById("bgCanvas");
  if (!canvas) return;

  const ctx = canvas.getContext("2d");
  let dpr = 1;
  let w = 0;
  let h = 0;

  const rand = mulberry32(1337);

  const stars = Array.from({ length: 220 }, () => ({
    x: rand(),
    y: rand(),
    r: 0.2 + rand() * 1.4,
    a: 0.12 + rand() * 0.45,
    tw: 0.25 + rand() * 1.25,
    hue: rand() < 0.18 ? "rgba(255,47,178," : rand() < 0.38 ? "rgba(167,150,120," : "rgba(140,210,255,",
  }));

  function resize() {
    dpr = Math.max(1, Math.floor(window.devicePixelRatio || 1));
    w = Math.max(1, Math.floor(window.innerWidth * dpr));
    h = Math.max(1, Math.floor(window.innerHeight * dpr));
    canvas.width = w;
    canvas.height = h;
  }

  let lastFrame = 0;
  function draw(now) {
    // Cap to ~45fps (background is purely decorative)
    if (now - lastFrame < 22) {
      requestAnimationFrame(draw);
      return;
    }
    lastFrame = now;

    const t = now / 1000;

    ctx.clearRect(0, 0, w, h);

    // Deep space base
    const bg = ctx.createLinearGradient(0, 0, 0, h);
    bg.addColorStop(0, "rgba(3,4,8,0.72)");
    bg.addColorStop(0.55, "rgba(2,3,6,0.72)");
    bg.addColorStop(1, "rgba(0,0,0,0.82)");
    ctx.fillStyle = bg;
    ctx.fillRect(0, 0, w, h);

    // Stars (subtle twinkle)
    for (const s of stars) {
      const tw = 0.55 + 0.45 * Math.sin(t * s.tw + s.x * 10.0);
      const a = s.a * tw;
      const x = s.x * w;
      const y = s.y * h;
      ctx.fillStyle = `${s.hue}${a.toFixed(3)})`;
      ctx.beginPath();
      ctx.arc(x, y, s.r * dpr, 0, Math.PI * 2);
      ctx.fill();
    }

    // Neon waves (very low alpha, behind the UI)
    ctx.save();
    ctx.globalCompositeOperation = "lighter";

    function wavePath(yNorm, amp, f1, f2, speed) {
      const baseY = h * yNorm;
      ctx.beginPath();
      for (let x = 0; x <= w; x += 14 * dpr) {
        const nx = x / w;
        const wobble =
          Math.sin((nx * Math.PI * 2) / f1 + t * speed) * amp +
          Math.sin((nx * Math.PI * 2) / f2 + t * (speed * 0.62)) * (amp * 0.45);
        const y = baseY + wobble * dpr;
        if (x === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
    }

    function strokeWave(yNorm, amp, f1, f2, speed, colour, glow) {
      wavePath(yNorm, amp, f1, f2, speed);
      ctx.lineWidth = 2.5 * dpr;
      ctx.strokeStyle = colour;
      ctx.shadowColor = glow;
      ctx.shadowBlur = 18 * dpr;
      ctx.stroke();

      wavePath(yNorm, amp, f1, f2, speed);
      ctx.lineWidth = 1.2 * dpr;
      ctx.shadowBlur = 8 * dpr;
      ctx.stroke();
      ctx.shadowBlur = 0;
    }

    strokeWave(0.28, 16, 0.52, 0.19, 0.9, "rgba(43,182,255,0.10)", "rgba(43,182,255,0.35)");
    strokeWave(0.46, 12, 0.44, 0.17, 0.68, "rgba(255,47,178,0.07)", "rgba(255,47,178,0.24)");
    strokeWave(0.68, 10, 0.58, 0.22, 0.52, "rgba(167,150,120,0.06)", "rgba(167,150,120,0.22)");

    ctx.restore();

    requestAnimationFrame(draw);
  }

  window.addEventListener("resize", resize);
  resize();
  requestAnimationFrame(draw);
}

function setPresetButtonText(text) {
  const el = document.getElementById("presetButtonText");
  if (el) el.textContent = text;
}

function getByPath(root, path) {
  const parts = path.split(".");
  let current = root;
  for (const part of parts) {
    current = current?.[part];
  }
  return current;
}

function setByPath(root, path, value) {
  const parts = path.split(".");
  let current = root;
  for (let i = 0; i < parts.length - 1; i++) {
    current = current[parts[i]];
  }
  current[parts.at(-1)] = value;
}

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}

function roundToStep(value, step) {
  const inv = 1 / step;
  return Math.round(value * inv) / inv;
}

function formatNumber(value) {
  if (Math.abs(value) >= 1000 && Number.isInteger(value)) return value.toLocaleString();
  if (Math.abs(value) < 10 && !Number.isInteger(value)) return value.toFixed(2);
  if (!Number.isInteger(value)) return value.toFixed(1);
  return String(value);
}

function hydrateSelects() {
  document.querySelectorAll('[data-bind$=".stack"]').forEach((select) => {
    select.innerHTML = "";
    for (const s of LAYER_STACKS) {
      const opt = document.createElement("option");
      opt.value = s;
      opt.textContent = s;
      select.appendChild(opt);
    }
  });
}

function makeSlider(el) {
  const min = Number(el.dataset.min);
  const max = Number(el.dataset.max);
  const step = Number(el.dataset.step);
  const unit = el.dataset.unit ?? "";

  const input = document.createElement("input");
  input.type = "range";
  input.min = String(min);
  input.max = String(max);
  input.step = String(step);

  const readout = document.createElement("div");
  readout.className = "readout";

  el.appendChild(input);
  el.appendChild(readout);

  return { input, readout, unit, min, max, step };
}

function makeKnob(el) {
  const min = Number(el.dataset.min);
  const max = Number(el.dataset.max);
  const step = Number(el.dataset.step);
  const unit = el.dataset.unit ?? "";

  const indicator = document.createElement("div");
  indicator.className = "indicator";
  const readout = document.createElement("div");
  readout.className = "readout";

  el.appendChild(indicator);
  el.appendChild(readout);

  let drag = null;

  function setVisual(value) {
    const t = (value - min) / (max - min);
    const angle = -135 + t * 270;
    indicator.style.transform = `translate(-50%, -90%) rotate(${angle}deg)`;
    readout.textContent = `${formatNumber(value)}${unit ? " " + unit : ""}`;
  }

  el.addEventListener("pointerdown", (e) => {
    el.setPointerCapture(e.pointerId);
    const startValue = Number(el.dataset.value ?? "0");
    drag = { startY: e.clientY, startValue };
  });
  el.addEventListener("pointermove", (e) => {
    if (!drag) return;
    const delta = (drag.startY - e.clientY) * 0.35; // pixels -> value weight
    const span = max - min;
    const nextRaw = drag.startValue + (delta / 100) * span;
    const next = clamp(roundToStep(nextRaw, step), min, max);
    el.dispatchEvent(new CustomEvent("knobchange", { detail: { value: next } }));
  });
  el.addEventListener("pointerup", () => {
    drag = null;
  });
  el.addEventListener("pointercancel", () => {
    drag = null;
  });

  return { setVisual, min, max, step, unit };
}

function renderAllControls() {
  document.querySelectorAll("[data-bind]").forEach((el) => {
    const bind = el.dataset.bind;

    if (el.classList.contains("toggle")) {
      el.checked = Boolean(getByPath(state, bind));
      return;
    }

    if (el.tagName === "SELECT") {
      el.value = String(getByPath(state, bind));
      return;
    }

    if (el.classList.contains("knob")) {
      const value = Number(getByPath(state, bind));
      el.dataset.value = String(value);
      el._knob?.setVisual(value);
      return;
    }

    if (el.classList.contains("slider")) {
      const value = Number(getByPath(state, bind));
      el._slider.input.value = String(value);
      el._slider.readout.textContent = `${formatNumber(value)}${el._slider.unit ? " " + el._slider.unit : ""}`;
      return;
    }
  });

  renderFxChain();
  drawMod();
}

function wireBindings() {
  document.querySelectorAll("[data-bind]").forEach((el) => {
    const bind = el.dataset.bind;

    if (el.classList.contains("toggle")) {
      el.addEventListener("change", () => {
        setByPath(state, bind, Boolean(el.checked));
        renderAllControls();
      });
      return;
    }

    if (el.tagName === "SELECT") {
      el.addEventListener("change", () => {
        setByPath(state, bind, el.value);
        renderAllControls();
      });
      return;
    }

    if (el.classList.contains("knob")) {
      el._knob = makeKnob(el);
      el.addEventListener("knobchange", (ev) => {
        const v = ev.detail.value;
        el.dataset.value = String(v);
        setByPath(state, bind, v);
        el._knob.setVisual(v);
        drawMod();
      });
      return;
    }

    if (el.classList.contains("slider")) {
      el._slider = makeSlider(el);
      el._slider.input.addEventListener("input", () => {
        const v = Number(el._slider.input.value);
        setByPath(state, bind, v);
        el._slider.readout.textContent = `${formatNumber(v)}${el._slider.unit ? " " + el._slider.unit : ""}`;
      });
      return;
    }
  });
}

function renderFxChain() {
  const list = document.getElementById("fxChain");
  list.innerHTML = "";

  const labels = {
    filter: "Filter (pre)",
    chorus: "Chorus",
    delay: "Delay",
    reverb: "Reverb",
    dist: "Distortion",
  };

  state.fx.chain.forEach((key, idx) => {
    const li = document.createElement("li");
    const name = document.createElement("div");
    name.textContent = labels[key] ?? key;

    const actions = document.createElement("div");
    actions.className = "chain-actions";

    const up = document.createElement("button");
    up.type = "button";
    up.textContent = "↑";
    up.disabled = idx === 0;
    up.addEventListener("click", () => {
      const next = [...state.fx.chain];
      [next[idx - 1], next[idx]] = [next[idx], next[idx - 1]];
      state.fx.chain = next;
      renderFxChain();
    });

    const down = document.createElement("button");
    down.type = "button";
    down.textContent = "↓";
    down.disabled = idx === state.fx.chain.length - 1;
    down.addEventListener("click", () => {
      const next = [...state.fx.chain];
      [next[idx + 1], next[idx]] = [next[idx], next[idx + 1]];
      state.fx.chain = next;
      renderFxChain();
    });

    actions.appendChild(up);
    actions.appendChild(down);
    li.appendChild(name);
    li.appendChild(actions);
    list.appendChild(li);
  });
}

function drawMod() {
  const canvas = document.getElementById("modCanvas");
  if (!canvas) return;
  const ctx = canvas.getContext("2d");
  const dpr = Math.max(1, Math.floor(window.devicePixelRatio || 1));
  const rect = canvas.getBoundingClientRect();

  const w = Math.floor(rect.width * dpr);
  const h = Math.floor(rect.height * dpr);
  if (canvas.width !== w || canvas.height !== h) {
    canvas.width = w;
    canvas.height = h;
  }

  ctx.clearRect(0, 0, w, h);

  // Grid
  ctx.globalAlpha = 1;
  ctx.lineWidth = 1 * dpr;
  ctx.strokeStyle = "rgba(43,182,255,0.10)";
  for (let x = 0; x <= 10; x++) {
    const px = (x / 10) * w;
    ctx.beginPath();
    ctx.moveTo(px, 0);
    ctx.lineTo(px, h);
    ctx.stroke();
  }
  ctx.strokeStyle = "rgba(167,150,120,0.08)";
  for (let y = 0; y <= 4; y++) {
    const py = (y / 4) * h;
    ctx.beginPath();
    ctx.moveTo(0, py);
    ctx.lineTo(w, py);
    ctx.stroke();
  }

  const shape = state.mod.shape;
  const amount = clamp(Number(state.mod.amount) / 100, 0, 1);

  const waveGrad = ctx.createLinearGradient(0, 0, w, 0);
  waveGrad.addColorStop(0, "rgba(43,182,255,0.75)");
  waveGrad.addColorStop(0.5, "rgba(167,150,120,0.65)");
  waveGrad.addColorStop(1, "rgba(255,47,178,0.75)");

  ctx.lineWidth = 2.2 * dpr;
  ctx.strokeStyle = waveGrad;
  ctx.shadowColor = "rgba(43,182,255,0.26)";
  ctx.shadowBlur = 14 * dpr;

  function evalShape(t) {
    if (shape === "sine") return Math.sin(t * Math.PI * 2) * 0.5 + 0.5;
    if (shape === "triangle") return 1 - Math.abs(((t * 2) % 2) - 1);
    if (shape === "square") return t < 0.5 ? 1 : 0;
    if (shape === "exp") return t === 0 ? 0 : Math.pow(t, 3);
    if (shape === "step4") return Math.floor(t * 4) / 3;
    return t;
  }

  ctx.beginPath();
  const fill = [];
  for (let i = 0; i <= 240; i++) {
    const t = i / 240;
    const v = evalShape(t);
    const y = h - ((0.15 + 0.7 * v) * amount + (1 - amount) * 0.5) * h;
    const x = t * w;
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
    fill.push([x, y]);
  }

  // Fill under curve (subtle)
  const fillGrad = ctx.createLinearGradient(0, 0, 0, h);
  fillGrad.addColorStop(0, "rgba(43,182,255,0.12)");
  fillGrad.addColorStop(0.65, "rgba(255,47,178,0.04)");
  fillGrad.addColorStop(1, "rgba(0,0,0,0)");
  ctx.fillStyle = fillGrad;
  ctx.beginPath();
  ctx.moveTo(0, h);
  for (const [x, y] of fill) ctx.lineTo(x, y);
  ctx.lineTo(w, h);
  ctx.closePath();
  ctx.fill();

  // Stroke curve
  ctx.beginPath();
  for (let i = 0; i < fill.length; i++) {
    const [x, y] = fill[i];
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }
  ctx.stroke();
  ctx.shadowBlur = 0;

  // Caption
  ctx.fillStyle = "rgba(242,238,232,0.65)";
  ctx.font = `${12 * dpr}px ui-sans-serif, system-ui`;
  const caption = `${state.mod.mode.toUpperCase()} • ${shape} • Rate ${formatNumber(Number(state.mod.rate))} • Amt ${formatNumber(Number(state.mod.amount))}%`;
  ctx.fillText(caption, 12 * dpr, 18 * dpr);
}

function initScopes() {
  const canvases = Array.from(document.querySelectorAll("canvas.scope"));
  if (!canvases.length) return;

  const scopes = canvases.map((canvas) => {
    const kind = canvas.dataset.scope;
    const layer = canvas.dataset.layer ?? null;
    const osc = canvas.dataset.osc ?? null;
    const seed = hashString(`${kind}:${layer ?? ""}:${osc ?? ""}`);
    const rand = mulberry32(seed);

    const bars = Array.from({ length: 56 }, () => 0.2 + Math.pow(rand(), 1.7) * 0.95);

    return {
      canvas,
      ctx: canvas.getContext("2d"),
      kind,
      layer,
      osc,
      rand,
      bars,
      phase: rand() * Math.PI * 2,
      dpr: 1,
      w: 0,
      h: 0,
    };
  });

  function resizeScopes() {
    const dpr = Math.max(1, Math.floor(window.devicePixelRatio || 1));
    for (const s of scopes) {
      const rect = s.canvas.getBoundingClientRect();
      const w = Math.max(1, Math.floor(rect.width * dpr));
      const h = Math.max(1, Math.floor(rect.height * dpr));
      if (s.canvas.width !== w || s.canvas.height !== h) {
        s.canvas.width = w;
        s.canvas.height = h;
      }
      s.dpr = dpr;
      s.w = w;
      s.h = h;
    }
  }

  function waveValue(type, t) {
    const u = (t % 1 + 1) % 1;
    if (type === "sine") return Math.sin(u * Math.PI * 2);
    if (type === "pulse") return u < 0.5 ? 1 : -1;
    if (type === "tri") return 2 * Math.abs(2 * (u - Math.floor(u + 0.5))) - 1;
    if (type === "noise") return (Math.random() * 2 - 1) * 0.85;
    if (type === "saw2") return Math.tanh((2 * u - 1) * 2.6);
    if (type === "saw3") return (2 * u - 1) * 0.78 + Math.sin(u * Math.PI * 2) * 0.22;
    if (type === "virus") return (2 * u - 1) * 0.7 + Math.sin(u * Math.PI * 2) * 0.3;
    if (type === "jp") return (2 * u - 1) * 0.65 + Math.sin(u * Math.PI * 4) * 0.22;
    return 2 * u - 1; // saw
  }

  function drawScope(scope, t) {
    const { ctx, w, h, dpr } = scope;
    if (!ctx || w <= 1 || h <= 1) return;

    ctx.clearRect(0, 0, w, h);

    // Screen background
    const bg = ctx.createLinearGradient(0, 0, 0, h);
    bg.addColorStop(0, "rgba(0,0,0,0.48)");
    bg.addColorStop(1, "rgba(0,0,0,0.22)");
    ctx.fillStyle = bg;
    ctx.fillRect(0, 0, w, h);

    // Border glow
    ctx.strokeStyle = "rgba(43,182,255,0.18)";
    ctx.lineWidth = 1 * dpr;
    ctx.strokeRect(1 * dpr, 1 * dpr, w - 2 * dpr, h - 2 * dpr);

    if (scope.kind === "stack") {
      const bars = scope.bars.length;
      const padX = 10 * dpr;
      const padY = 10 * dpr;
      const innerW = w - padX * 2;
      const innerH = h - padY * 2;
      const step = innerW / bars;
      const barW = Math.max(1, step * 0.62);

      const grad = ctx.createLinearGradient(padX, 0, padX + innerW, 0);
      grad.addColorStop(0, "rgba(43,182,255,0.85)");
      grad.addColorStop(0.55, "rgba(167,150,120,0.55)");
      grad.addColorStop(1, "rgba(255,47,178,0.75)");

      ctx.save();
      ctx.globalCompositeOperation = "lighter";
      ctx.shadowColor = "rgba(43,182,255,0.22)";
      ctx.shadowBlur = 12 * dpr;
      ctx.fillStyle = grad;

      for (let i = 0; i < bars; i++) {
        const base = scope.bars[i];
        const wob = 0.55 + 0.45 * Math.sin(t * 1.25 + i * 0.35 + scope.phase);
        const v = clamp(base * wob, 0, 1);
        const bh = v * innerH;
        const x = padX + i * step + (step - barW) * 0.5;
        const y = padY + (innerH - bh);
        ctx.fillRect(x, y, barW, bh);
      }

      // Sparkline overlay
      ctx.shadowBlur = 0;
      ctx.lineWidth = 1.6 * dpr;
      ctx.strokeStyle = "rgba(255,255,255,0.10)";
      ctx.beginPath();
      for (let i = 0; i < bars; i++) {
        const base = scope.bars[i];
        const wob = 0.55 + 0.45 * Math.sin(t * 1.25 + i * 0.35 + scope.phase);
        const v = clamp(base * wob, 0, 1);
        const bh = v * innerH;
        const x = padX + i * step + step * 0.5;
        const y = padY + (innerH - bh);
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();

      ctx.restore();
      return;
    }

    // Oscillator waveform
    const oscIndex = Number(scope.osc);
    const oscState = state.osc?.[oscIndex];
    const type = oscState?.wave ?? "saw";

    const grad = ctx.createLinearGradient(0, 0, w, 0);
    grad.addColorStop(0, "rgba(43,182,255,0.78)");
    grad.addColorStop(0.5, "rgba(167,150,120,0.55)");
    grad.addColorStop(1, "rgba(255,47,178,0.78)");

    ctx.save();
    ctx.globalCompositeOperation = "lighter";

    const mid = h * 0.52;
    const amp = h * 0.30;
    const points = 180;
    const phase = t * 0.22 + scope.phase;

    ctx.lineWidth = 2.2 * dpr;
    ctx.strokeStyle = grad;
    ctx.shadowColor = "rgba(43,182,255,0.25)";
    ctx.shadowBlur = 14 * dpr;

    ctx.beginPath();
    for (let i = 0; i <= points; i++) {
      const x = (i / points) * w;
      const u = i / points;
      const v = waveValue(type, u + phase);
      const y = mid + v * amp * (0.82 + 0.18 * Math.sin(t * 0.9 + scope.phase));
      if (i === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    }
    ctx.stroke();

    // Highlight pass
    ctx.shadowBlur = 0;
    ctx.lineWidth = 1.0 * dpr;
    ctx.strokeStyle = "rgba(255,255,255,0.16)";
    ctx.stroke();

    // Moving sparkle
    const dotU = (phase * 0.22) % 1;
    const dotX = dotU * w;
    const dotV = waveValue(type, dotU + phase);
    const dotY = mid + dotV * amp;
    ctx.fillStyle = "rgba(255,255,255,0.55)";
    ctx.shadowColor = "rgba(43,182,255,0.45)";
    ctx.shadowBlur = 18 * dpr;
    ctx.beginPath();
    ctx.arc(dotX, dotY, 2.2 * dpr, 0, Math.PI * 2);
    ctx.fill();

    ctx.restore();
  }

  let lastFrame = 0;
  function tick(now) {
    if (now - lastFrame >= 33) {
      lastFrame = now;
      const t = now / 1000;
      for (const s of scopes) drawScope(s, t);
    }
    requestAnimationFrame(tick);
  }

  resizeScopes();
  requestAnimationFrame(tick);

  scopeState = { resizeScopes };
}

function randomize() {
  function rand(min, max) {
    return min + Math.random() * (max - min);
  }
  function rint(min, max) {
    return Math.round(rand(min, max));
  }

  const next = structuredClone(state);
  next.osc[1].voices = rint(8, 32);
  next.osc[2].voices = rint(8, 32);
  next.osc[3].voices = rint(4, 24);
  next.osc[1].detune = rint(15, 95);
  next.osc[2].detune = rint(15, 95);
  next.osc[3].detune = rint(0, 75);
  next.fx.reverb.mix = rint(0, 35);
  next.fx.delay.mix = rint(0, 35);
  next.fx.chorus.mix = rint(0, 65);
  next.fx.dist.drive = rint(0, 55);
  next.layers.A.startRand = rint(0, 90);
  next.layers.B.startRand = rint(0, 90);
  next.mod.amount = rint(0, 80);
  next.mod.rate = roundToStep(rand(0.1, 8.0), 0.05);
  state = next;
  setPresetButtonText("Randomized (demo)");
  renderAllControls();
}

function startMeter() {
  const bar = document.getElementById("meterBar");
  if (!bar) return;
  setInterval(() => {
    const t = Date.now() / 600;
    const wobble = 0.22 * Math.sin(t) + 0.12 * Math.sin(t * 1.93);
    const base = 0.35 + 0.18 * (Number(state.master.volume) / 100);
    const width = clamp(base + wobble, 0.06, 0.98);
    bar.style.width = `${Math.round(width * 100)}%`;
  }, 80);
}

function setupTopButtons() {
  document.getElementById("randomizeBtn").addEventListener("click", randomize);
  document.getElementById("resetBtn").addEventListener("click", () => {
    state = structuredClone(DEFAULT_STATE);
    currentPresetIndex = 0;
    setPresetButtonText(PRESETS[0].name);
    renderAllControls();
  });
}

function setupModButtons() {
  const shapes = ["sine", "triangle", "square", "exp", "step4"];
  document.getElementById("modShapeBtn").addEventListener("click", () => {
    const idx = shapes.indexOf(state.mod.shape);
    state.mod.shape = shapes[(idx + 1) % shapes.length];
    drawMod();
  });

  document.getElementById("modTriggerBtn").addEventListener("click", () => {
    // Cosmetic “trigger” pulse: briefly animate amount.
    const base = Number(state.mod.amount);
    state.mod.amount = clamp(base + 22, 0, 100);
    drawMod();
    setTimeout(() => {
      state.mod.amount = base;
      drawMod();
    }, 180);
  });
}

function categorizePresetName(name) {
  const prefix = name.split("—")[0].trim().toLowerCase();
  if (prefix.startsWith("init")) return { group: "Init", tag: "Base" };
  if (prefix.startsWith("rave")) return { group: "Rave", tag: "Hard Dance" };
  if (prefix.startsWith("trance")) return { group: "Trance", tag: "Pluck" };
  return { group: "Other", tag: "User" };
}

function buildPresetBrowser() {
  const picker = document.getElementById("presetPicker");
  const btn = document.getElementById("presetButton");
  const btnText = document.getElementById("presetButtonText");
  const menu = document.getElementById("presetMenu");
  const body = document.getElementById("presetMenuBody");
  const search = document.getElementById("presetSearch");
  const close = document.getElementById("presetClose");

  function setOpen(nextOpen) {
    btn.setAttribute("aria-expanded", nextOpen ? "true" : "false");
    menu.hidden = !nextOpen;
    if (nextOpen) {
      search.value = "";
      renderMenu("");
      search.focus();
    }
  }

  function applyPreset(index) {
    currentPresetIndex = index;
    const preset = PRESETS[index];
    state = structuredClone(preset.state);
    btnText.textContent = preset.name;
    renderAllControls();
  }

  function renderMenu(query) {
    const q = query.trim().toLowerCase();
    const items = PRESETS.map((p, i) => ({ preset: p, index: i, ...categorizePresetName(p.name) })).filter((x) =>
      q ? x.preset.name.toLowerCase().includes(q) : true,
    );

    const groups = new Map();
    for (const it of items) {
      if (!groups.has(it.group)) groups.set(it.group, []);
      groups.get(it.group).push(it);
    }

    body.innerHTML = "";
    for (const [group, groupItems] of groups.entries()) {
      const wrap = document.createElement("div");
      wrap.className = "preset-group";

      const title = document.createElement("div");
      title.className = "preset-group-title";
      title.textContent = group;

      wrap.appendChild(title);
      for (const it of groupItems) {
        const row = document.createElement("div");
        row.className = "preset-item";
        row.tabIndex = 0;
        row.setAttribute("role", "button");
        row.setAttribute("aria-label", `Load preset: ${it.preset.name}`);

        const name = document.createElement("div");
        name.className = "preset-item-name";
        name.textContent = it.preset.name;

        const tag = document.createElement("div");
        tag.className = "preset-item-tag";
        tag.textContent = it.tag;

        row.appendChild(name);
        row.appendChild(tag);
        row.addEventListener("click", () => {
          applyPreset(it.index);
          setOpen(false);
        });
        row.addEventListener("keydown", (e) => {
          if (e.key === "Enter" || e.key === " ") {
            e.preventDefault();
            applyPreset(it.index);
            setOpen(false);
          }
        });

        wrap.appendChild(row);
      }
      body.appendChild(wrap);
    }

    if (!items.length) {
      const empty = document.createElement("div");
      empty.className = "hint";
      empty.textContent = "No presets match your search.";
      body.appendChild(empty);
    }
  }

  btn.addEventListener("click", () => setOpen(menu.hidden));
  close.addEventListener("click", () => setOpen(false));
  search.addEventListener("input", () => renderMenu(search.value));

  document.addEventListener("click", (e) => {
    if (menu.hidden) return;
    const path = e.composedPath?.() ?? [];
    if (path.includes(picker)) return;
    setOpen(false);
  });
  document.addEventListener("keydown", (e) => {
    if (e.key === "Escape" && !menu.hidden) setOpen(false);
  });

  applyPreset(currentPresetIndex);
}

function init() {
  initBackground();
  initScopes();
  hydrateSelects();
  wireBindings();
  setupTopButtons();
  setupModButtons();
  buildPresetBrowser();

  renderAllControls();
  startMeter();

  window.addEventListener("resize", () => {
    drawMod();
    scopeState?.resizeScopes?.();
  });
}

init();
