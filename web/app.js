// {Shan, Shui}* C++/WASM port - UI glue
"use strict";

const WINDX = 3000;
let cursx = 0;
let SS = null;
let btnHoverCol = "rgba(0,0,0,0.1)";

function update() {
  const svg = SS.ss_get_view(cursx, WINDX);
  document.getElementById("BG").innerHTML = svg;
}

function xcroll(v) {
  cursx += v;
  update();
}

function autoxcroll(v) {
  if (document.getElementById("AUTO_SCROLL").checked) {
    xcroll(v);
    setTimeout(function () { autoxcroll(v); }, 2000);
  }
}

function rstyle(id, b) {
  const a = b ? 0.1 : 0.0;
  const el = document.getElementById(id);
  el.setAttribute("style",
    "width:32px; text-align:center; top:0px; color:rgba(0,0,0,0.4);" +
    "display:table; cursor:pointer; border:1px solid rgba(0,0,0,0.4);" +
    "background-color:rgba(0,0,0," + a + "); height:800px");
  document.getElementById(id + ".t").setAttribute("style",
    "vertical-align:middle; display:table-cell");
}

function toggleVisible(id) {
  const v = document.getElementById(id).style.display == "none";
  document.getElementById(id).style.display = v ? "block" : "none";
}

function toggleText(id, a, b) {
  const el = document.getElementById(id);
  el.innerHTML = el.innerHTML == "" || el.innerHTML == b ? a : b;
}

function reloadWSeed(s) {
  const u = window.location.href.split("?")[0];
  window.location.href = u + "?seed=" + s;
}

function download(filename, text) {
  const element = document.createElement("a");
  element.setAttribute("href", "data:text/plain;charset=utf-8," + encodeURIComponent(text));
  element.setAttribute("download", filename);
  element.style.display = "none";
  document.body.appendChild(element);
  element.click();
  document.body.removeChild(element);
}

// paper texture background (uses the wasm's perlin noise)
function makePaperTexture() {
  const canvas = document.getElementById("bgcanv");
  const ctx = canvas.getContext("2d");
  const reso = 512;
  for (let i = 0; i < reso / 2 + 1; i++) {
    for (let j = 0; j < reso / 2 + 1; j++) {
      let c = 245 + SS.ss_noise(i * 0.1, j * 0.1) * 10;
      c -= Math.random() * 20;
      const r = c.toFixed(0);
      const g = (c * 0.95).toFixed(0);
      const b = (c * 0.85).toFixed(0);
      ctx.fillStyle = "rgb(" + r + "," + g + "," + b + ")";
      ctx.fillRect(i, j, 1, 1);
      ctx.fillRect(reso - i, j, 1, 1);
      ctx.fillRect(i, reso - j, 1, 1);
      ctx.fillRect(reso - i, reso - j, 1, 1);
    }
  }
  const img = canvas.toDataURL("image/png");
  document.getElementById("BG").style.backgroundImage = "url(" + img + ")";
  document.getElementsByTagName("body")[0].style.backgroundImage = "url(" + img + ")";
}

async function main() {
  SS = await createSS();
  const par = new URLSearchParams(window.location.search);
  const seed = par.get("seed") || "" + new Date().getTime();
  SS.ss_init(seed);
  document.getElementById("INP_SEED").value = seed;
  document.getElementById("BG").setAttribute("style", "width:" + WINDX + "px");
  rstyle("L", false);
  rstyle("R", false);
  makePaperTexture();
  update();
  document.body.scrollTo(0, 0);
}

main();
