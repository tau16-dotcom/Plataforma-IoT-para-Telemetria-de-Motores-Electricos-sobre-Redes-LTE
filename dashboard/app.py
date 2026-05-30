import streamlit as st
import requests
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from datetime import datetime
from zoneinfo import ZoneInfo
import time
import numpy as np

# ─────────────────────────────────────────
API_URL          = "http://3.150.75.105:3000"
REFRESH_SECONDS  = 5
TIMEOUT_SIN_DATO = 30
ZONA_HORARIA     = ZoneInfo("America/Bogota")
# ─────────────────────────────────────────

st.set_page_config(
    page_title="Monitor Industrial",
    page_icon="🔧",
    layout="wide",
    initial_sidebar_state="collapsed"
)

st.markdown("""
<style>
@import url('https://fonts.googleapis.com/css2?family=Rajdhani:wght@300;400;500;600;700&family=Share+Tech+Mono&display=swap');

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
html, body, .main, .block-container { background-color: #080c10 !important; color: #c8d8e8; }
.block-container { padding: 1.5rem 2rem !important; max-width: 100% !important; }
#MainMenu, footer, header { visibility: hidden; }
[data-testid="stToolbar"] { display: none; }
body { font-family: 'Rajdhani', sans-serif; }
h1, h2, h3 { font-family: 'Share Tech Mono', monospace; }

.hdr {
    display: flex; align-items: center; justify-content: space-between;
    border-bottom: 1px solid #1a2a3a; padding-bottom: 1rem; margin-bottom: 1.5rem;
}
.hdr-title { font-family: 'Share Tech Mono', monospace; font-size: 1.1rem; color: #4af; letter-spacing: 4px; text-transform: uppercase; }
.hdr-sub { font-size: 0.75rem; color: #446; letter-spacing: 2px; margin-top: 2px; }
.hdr-right { text-align: right; font-family: 'Share Tech Mono', monospace; font-size: 0.7rem; color: #446; letter-spacing: 1px; }

.badge { display: inline-flex; align-items: center; gap: 6px; border-radius: 3px; padding: 4px 10px; font-family: 'Share Tech Mono', monospace; font-size: 0.65rem; letter-spacing: 2px; margin-bottom: 1.5rem; }
.badge-online  { background:#0a1520; border:1px solid #1a3050; color:#3a9; }
.badge-offline { background:#1a0a00; border:1px solid #432;    color:#f80; }
.badge-noapi   { background:#1a0000; border:1px solid #522;    color:#f44; }

.dot { width:6px; height:6px; border-radius:50%; animation:blink 1.4s ease-in-out infinite; }
.dot-green  { background:#3a9; box-shadow:0 0 6px #3a9; }
.dot-orange { background:#f80; box-shadow:0 0 6px #f80; }
.dot-red    { background:#f44; box-shadow:0 0 6px #f44; }
@keyframes blink { 0%,100%{opacity:1} 50%{opacity:0.2} }

.metric-wrap {
    background: #0a1520; border: 1px solid #1a2a3a;
    border-top: 2px solid var(--accent); border-radius: 4px;
    padding: 1.4rem 1.6rem; position: relative; overflow: hidden;
}
.metric-wrap::before {
    content:''; position:absolute; top:0;left:0;right:0;bottom:0;
    background:linear-gradient(135deg,rgba(68,170,255,0.04) 0%,transparent 60%);
    pointer-events:none;
}
.metric-tag { font-family:'Share Tech Mono',monospace; font-size:0.6rem; letter-spacing:3px; color:#446; text-transform:uppercase; margin-bottom:0.8rem; }
.metric-val { font-family:'Share Tech Mono',monospace; font-size:2.8rem; font-weight:400; color:var(--accent); line-height:1; text-shadow:0 0 20px var(--accent-glow); }
.metric-unit { font-size:1rem; color:#446; margin-left:4px; }
.metric-ts { font-size:0.65rem; color:#334; margin-top:0.5rem; font-family:'Share Tech Mono',monospace; }
.corner { position:absolute; bottom:8px; right:10px; font-family:'Share Tech Mono',monospace; font-size:0.55rem; color:#223; letter-spacing:1px; }

.divider { font-family:'Share Tech Mono',monospace; font-size:0.6rem; letter-spacing:4px; color:#223; text-transform:uppercase; border-top:1px solid #111a22; padding-top:0.6rem; margin:1.8rem 0 1rem 0; }

[data-testid="stSidebar"] { background:#080c10 !important; border-right:1px solid #1a2a3a !important; }
[data-testid="stSidebar"] * { color:#4af !important; }
</style>
""", unsafe_allow_html=True)


def get_latest() -> dict:
    try:
        r = requests.get(f"{API_URL}/api/latest", timeout=4)
        return r.json()
    except Exception:
        return {}

def get_historical(n: int = 300) -> pd.DataFrame:
    try:
        r = requests.get(f"{API_URL}/api/history?n={n}", timeout=6)
        data = r.json()
        if not data:
            return pd.DataFrame()
        df = pd.DataFrame(data)
        df["timestamp"] = pd.to_datetime(df["timestamp"])
        for col in ["temp_ambiente", "temp_objeto", "corriente"]:
            if col in df.columns:
                df[col] = pd.to_numeric(df[col], errors="coerce")
        return df
    except Exception:
        return pd.DataFrame()

def segundos_desde_ultima(ts_str: str) -> float:
    try:
        ts = datetime.strptime(ts_str, "%Y-%m-%d %H:%M:%S")
        ahora = datetime.now(ZONA_HORARIA).replace(tzinfo=None)
        return (ahora - ts).total_seconds()
    except Exception:
        return 9999

def make_chart(df: pd.DataFrame) -> go.Figure:
    fig = make_subplots(
        rows=2, cols=1,
        shared_xaxes=True,
        row_heights=[0.6, 0.4],
        vertical_spacing=0.08,
    )

    fig.add_trace(go.Scatter(
        x=df["timestamp"], y=df["temp_ambiente"],
        name="T. Ambiente", mode="lines",
        line=dict(color="#44aaff", width=1.5),
        hovertemplate="%{y:.1f}°C<extra>Ambiente</extra>"
    ), row=1, col=1)

    fig.add_trace(go.Scatter(
        x=df["timestamp"], y=df["temp_objeto"],
        name="T. Objeto", mode="lines",
        line=dict(color="#ff6655", width=1.5),
        hovertemplate="%{y:.1f}°C<extra>Objeto</extra>"
    ), row=1, col=1)

    if "corriente" in df.columns and df["corriente"].notna().any():
        fig.add_trace(go.Scatter(
            x=df["timestamp"], y=df["corriente"],
            name="Corriente", mode="lines",
            line=dict(color="#33cc88", width=1.5),
            fill="tozeroy", fillcolor="rgba(51,204,136,0.06)",
            hovertemplate="%{y:.2f} A<extra>Corriente</extra>"
        ), row=2, col=1)

    BG, PLOT, GRID, TXT = "#080c10", "#0a1520", "#111a24", "#334455"
    fig.update_layout(
        paper_bgcolor=BG, plot_bgcolor=PLOT,
        font=dict(family="Share Tech Mono", color=TXT, size=10),
        legend=dict(bgcolor="#0a1520", bordercolor="#1a2a3a", borderwidth=1,
                    font=dict(size=10), orientation="h",
                    yanchor="bottom", y=1.01, xanchor="left", x=0),
        margin=dict(l=0, r=0, t=30, b=0), height=480,
        hovermode="x unified",
        hoverlabel=dict(bgcolor="#0a1520", bordercolor="#1a2a3a",
                        font=dict(family="Share Tech Mono", size=11, color="#4af")),
    )
    fig.update_xaxes(gridcolor=GRID, linecolor=GRID, zeroline=False)
    fig.update_yaxes(gridcolor=GRID, linecolor=GRID, zeroline=False)
    return fig


def main():
    try:
        requests.get(f"{API_URL}/api/latest", timeout=3)
        api_ok = True
    except Exception:
        api_ok = False

    now = datetime.now(ZONA_HORARIA).strftime("%Y-%m-%d  %H:%M:%S")

    st.markdown(f"""
    <div class="hdr">
        <div>
            <div class="hdr-title">⬡ Monitor Industrial</div>
            <div class="hdr-sub">ESP32 · GY-906 · SCT-013</div>
        </div>
        <div class="hdr-right">{now}<br>SISTEMA ACTIVO</div>
    </div>
    """, unsafe_allow_html=True)

    latest = get_latest() if api_ok else {}
    ts = latest.get("timestamp", "--")

    if not api_ok:
        badge_html = '<div class="badge badge-noapi"><span class="dot dot-red"></span>SIN CONEXIÓN A SERVIDOR</div>'
    else:
        segundos = segundos_desde_ultima(ts)
        if segundos > TIMEOUT_SIN_DATO:
            badge_html = (f'<div class="badge badge-offline"><span class="dot dot-orange"></span>'
                          f'SIN SEÑAL · ÚLTIMA LECTURA HACE {int(segundos)}s</div>')
        else:
            badge_html = (f'<div class="badge badge-online"><span class="dot dot-green"></span>'
                          f'EN LÍNEA · ACTUALIZA CADA {REFRESH_SECONDS}s</div>')

    st.markdown(badge_html, unsafe_allow_html=True)

    with st.sidebar:
        st.markdown("### CONFIG")
        n_lecturas = st.slider("Lecturas en gráfica", 50, 500, 300)

    st.markdown('<div class="divider">── Lecturas actuales ──────────────────</div>', unsafe_allow_html=True)
    c1, c2, c3 = st.columns(3)

    def val_str(v, fmt):
        try:    return format(float(v), fmt)
        except: return "---"

    with c1:
        v = latest.get("temp_ambiente", "--")
        st.markdown(f"""
        <div class="metric-wrap" style="--accent:#44aaff; --accent-glow:rgba(68,170,255,0.3)">
            <div class="metric-tag">// temp_ambiente</div>
            <div class="metric-val">{val_str(v,'.1f')}<span class="metric-unit">°C</span></div>
            <div class="metric-ts">{ts}</div>
            <div class="corner">SENSOR_01</div>
        </div>""", unsafe_allow_html=True)

    with c2:
        v = latest.get("temp_objeto", "--")
        st.markdown(f"""
        <div class="metric-wrap" style="--accent:#ff6655; --accent-glow:rgba(255,100,80,0.3)">
            <div class="metric-tag">// temp_objeto</div>
            <div class="metric-val">{val_str(v,'.1f')}<span class="metric-unit">°C</span></div>
            <div class="metric-ts">{ts}</div>
            <div class="corner">SENSOR_02</div>
        </div>""", unsafe_allow_html=True)

    with c3:
        v = latest.get("corriente", None)
        val_display = val_str(v, '.2f') if v is not None else "---"
        accent = "#33cc88" if v is not None else "#223344"
        glow   = "rgba(51,204,136,0.3)" if v is not None else "rgba(0,0,0,0)"
        st.markdown(f"""
        <div class="metric-wrap" style="--accent:{accent}; --accent-glow:{glow}">
            <div class="metric-tag">// corriente</div>
            <div class="metric-val">{val_display}<span class="metric-unit">A</span></div>
            <div class="metric-ts">{ts}</div>
            <div class="corner">SENSOR_03</div>
        </div>""", unsafe_allow_html=True)

    st.markdown('<div class="divider">── Historial ──────────────────────────</div>', unsafe_allow_html=True)
    df = get_historical(n_lecturas) if api_ok else pd.DataFrame()

    if not df.empty:
        st.plotly_chart(make_chart(df), use_container_width=True)

        st.markdown('<div class="divider">── Registros recientes ────────────────</div>', unsafe_allow_html=True)
        df_show = df.sort_values("timestamp", ascending=False).copy()
        df_show["timestamp"] = df_show["timestamp"].dt.strftime("%Y-%m-%d %H:%M:%S")
        cols = ["timestamp", "temp_ambiente", "temp_objeto", "corriente"]
        cols_ok = [c for c in cols if c in df_show.columns]
        df_show = df_show[cols_ok].rename(columns={
            "timestamp":     "Fecha/Hora",
            "temp_ambiente": "T. Ambiente (°C)",
            "temp_objeto":   "T. Objeto (°C)",
            "corriente":     "Corriente (A)",
        })
        st.dataframe(df_show, use_container_width=True, height=280, hide_index=True)
    else:
        st.info("Sin datos históricos aún.")

    time.sleep(REFRESH_SECONDS)
    st.rerun()


if __name__ == "__main__":
    main()
