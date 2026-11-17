"""Screenshots of the interface for README.md / README.en.md (docs/images/<page>-zh|en.webp).

Builds the demo project of the primer tutorial (six solenoid + HWR periods, 2 mA, 20 000 macro-particles) in a
temporary folder, runs it with the new kernel on the CPU, starts ``avas serve`` and drives a headless Chrome / Edge
over the DevTools protocol (light theme, 1440 x 900 viewport at 1.5x).  Local paths are replaced by ~/AVAS/demo
before each capture.  Nothing outside the temporary folder and docs/images is written.

    python tools/readme_screenshots.py                 # both languages, every page
    python tools/readme_screenshots.py --lang en --pages lattice run
    python tools/readme_screenshots.py --browser "C:/Program Files/Google/Chrome/Application/chrome.exe"

Needs Pillow (with WebP) and websockets, both dependencies of AVAS.  Labels are looked up in the interface's own
text, so a renamed button needs an update here.
"""
import argparse
import asyncio
import base64
import io
import json
import os
import shutil
import socket
import subprocess
import sys
import tempfile
import time
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
OUT = os.path.join(ROOT, "docs", "images")
W, H, DPR = 1440, 900, 1.5
TOKEN = "readme"
BROWSERS = [
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
    "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge",
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    "/usr/bin/google-chrome", "/usr/bin/chromium", "/usr/bin/chromium-browser",
]

HELPERS = r"""
window.__sleep = (ms) => new Promise(r => setTimeout(r, ms));
window.__byText = (t) => [...document.querySelectorAll("button, [role=tab], .files-item, label, a, .tab")]
  .filter(e => e.offsetParent !== null)
  .find(e => e.textContent.trim() === t || e.getAttribute("data-tip") === t || e.getAttribute("aria-label") === t);
window.__click = async (t, ms = 800) => { const e = __byText(t); if (!e) throw new Error("no element " + t); e.click(); await __sleep(ms); };
window.__menuPick = async (opener, item) => { await __click(opener, 500);
  const leaf = [...document.querySelectorAll("body *")].filter(e => e.offsetParent !== null && e.children.length === 0 && e.textContent.trim() === item).pop();
  if (!leaf) throw new Error("no menu item " + item);
  (leaf.closest("[role=menuitem], [role=menuitemcheckbox], button, .menu-item, li") || leaf).click(); await __sleep(600);
  document.dispatchEvent(new KeyboardEvent("keydown", {key: "Escape"})); await __sleep(300); };
window.__scrub = (path) => { const w = document.createTreeWalker(document.body, NodeFilter.SHOW_TEXT);
  const fix = (t) => t.split(path).join("~/AVAS/demo").split(path.replace(/\//g, "\\")).join("~/AVAS/demo");
  for (let n = w.nextNode(); n; n = w.nextNode()) if (n.nodeValue.includes(path) || n.nodeValue.includes(path.replace(/\//g, "\\"))) n.nodeValue = fix(n.nodeValue);
  document.querySelectorAll("[data-tip]").forEach(e => e.setAttribute("data-tip", fix(e.getAttribute("data-tip")))); };
window.__rpc = (method, params) => fetch("/api/rpc", {method: "POST", headers: {"Content-Type": "application/json", "X-AVAS-Token": TOKEN},
  body: JSON.stringify({method, params})}).then(r => r.json());
""".replace("TOKEN", json.dumps(TOKEN))


def shots(T):
    """{page name: [JavaScript steps]} for one language (*T(zh, en)* picks the label)."""
    j = json.dumps

    def page(p, ms=2500):
        return f"window.__avasDebug.setPage({j(p)}); await __sleep({ms});"
    return {
        "lattice": [page("lattice"), f"""await __click({j(T('可视化编辑器', 'Visual editor'))}, 2500);
            await __menuPick({j(T('叠加显示', 'Overlays'))}, {j(T('上次运行', 'Last run'))});
            await __menuPick({j(T('叠加显示', 'Overlays'))}, {j(T('能量（右轴）', 'Energy (right axis)'))});
            const row = [...document.querySelectorAll("tr, .tree-row, [role=row]")]
              .filter(e => e.offsetParent !== null && e.textContent.includes({j(T('射频腔', 'RF cavity'))}))[1];
            if (row) row.click(); await __sleep(2500);"""],
        "lattice3d": [f"await __click({j(T('三维视图', 'Three-dimensional view'))}, 4000);"],
        "run": [page("run"), f"await __click({j(T('回放这条记录', 'Replay this record'))}, 7000);"],
        "results": [page("results"), f"""await __click({j(T('粒子文件查看器 (.dst)', 'Particle file viewer (.dst)'))}, 4000);
            const sel = [...document.querySelectorAll("select")].find(x => [...x.options].some(o => o.textContent.includes("outData_")));
            if (sel) {{ const o = [...sel.options].filter(o => o.textContent.includes("outData_")).pop();
              Object.getOwnPropertyDescriptor(HTMLSelectElement.prototype, "value").set.call(sel, o.value);
              sel.dispatchEvent(new Event("change", {{bubbles: true}})); await __sleep(4000); }}"""],
        "learn": [page("learn", 3000), f"""await __click({j(T('螺线管', 'Solenoid'))}, 3500);
            const lab = document.querySelector(".learn-lab"); if (lab) lab.scrollIntoView({{block: "start"}});
            let el = lab && lab.parentElement; while (el && el.scrollHeight <= el.clientHeight + 1) el = el.parentElement;
            if (el) el.scrollTop -= 110; await __sleep(2500);"""],
    }


def demo_project(folder):
    from avas.learn import tutorial
    project = os.path.join(folder, "demo")
    tutorial.export(project, "current", {"periods": 6, "I": 2, "particles": 20000})
    with open(os.path.join(project, "InputFile", "ini.ini"), "a", encoding="utf-8") as fh:
        fh.write("\n[engine]\nkernel = new\ndevice = cpu\n")
    subprocess.run([sys.executable, "-m", "avas", "run", "--input", os.path.join(project, "InputFile"),
                    "--output", os.path.join(project, "OutputFile")], cwd=ROOT, check=True)
    return project


def free_port():
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


async def capture(browser, url, project, langs, pages):
    import websockets
    from PIL import Image
    port = free_port()
    profile = tempfile.mkdtemp(prefix="avas-readme-chrome-")
    proc = subprocess.Popen([browser, "--headless=new", f"--remote-debugging-port={port}", f"--user-data-dir={profile}",
                             "--no-first-run", "--no-default-browser-check", "--hide-scrollbars", "--ignore-gpu-blocklist",
                             f"--window-size={W},{H}", "about:blank"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        for _ in range(100):
            try:
                tabs = json.load(urllib.request.urlopen(f"http://127.0.0.1:{port}/json"))
                target = next(t for t in tabs if t["type"] == "page")
                break
            except Exception:  # noqa: BLE001 - the browser is still starting
                time.sleep(0.2)
        else:
            raise RuntimeError("the browser did not start")
        async with websockets.connect(target["webSocketDebuggerUrl"], max_size=2 ** 28) as ws:
            seq = 0

            async def send(method, **params):
                nonlocal seq
                seq += 1
                me = seq
                await ws.send(json.dumps({"id": me, "method": method, "params": params}))
                while True:
                    msg = json.loads(await ws.recv())
                    if msg.get("id") == me:
                        if "error" in msg:
                            raise RuntimeError(f"{method}: {msg['error']}")
                        return msg.get("result", {})

            async def js(code):
                r = await send("Runtime.evaluate", expression=f"(async () => {{ {code} }})()", awaitPromise=True,
                               returnByValue=True)
                if r.get("exceptionDetails"):
                    print("  script error:", r["exceptionDetails"].get("exception", {}).get("description", "?").splitlines()[0])
                    t = await send("Runtime.evaluate", expression="document.body.innerText.slice(0, 300)", returnByValue=True)
                    print("  page shows:", " | ".join(str(t.get("result", {}).get("value", "")).split()))

            await send("Emulation.setDeviceMetricsOverride", width=W, height=H, deviceScaleFactor=DPR, mobile=False)
            await send("Emulation.setEmulatedMedia", features=[{"name": "prefers-color-scheme", "value": "light"},
                                                               {"name": "prefers-reduced-motion", "value": "no-preference"}])
            for lang in langs:
                T = (lambda zh, en: zh) if lang == "zh" else (lambda zh, en: en)
                # the same starting state for every language: view choices live in the page's localStorage
                await send("Page.navigate", url=url)
                await asyncio.sleep(2)
                await send("Runtime.evaluate", expression="localStorage.clear()")
                await send("Page.navigate", url=url)
                await asyncio.sleep(4)
                await send("Runtime.evaluate", expression=HELPERS)
                await js("""for (let i = 0; i < 150; i++) {
                    if (window.__avasDebug && window.__avasDebug.useApp.getState().ready) break; await __sleep(200); }""")
                await js(f"""for (let i = 0; i < 50 && !window.__avasDebug.useApp.getState().project.open; i++) await __sleep(200);
                    await __sleep(1000);
                    window.__avasDebug.setLanguage({json.dumps('zh_CN' if lang == 'zh' else 'en')});
                    window.__avasDebug.setTheme("light"); await __sleep(800);
                    const hide = __byText({json.dumps(T('隐藏面板 (Ctrl+J)', 'Hide panel (Ctrl+J)'))}); if (hide) hide.click();
                    await __sleep(500);""")
                for name, steps in shots(T).items():
                    if pages and name not in pages:
                        continue
                    for step in steps:
                        await js(step)
                    for path in {project, os.path.realpath(project)}:
                        await js(f"__scrub({json.dumps(path.replace(os.sep, '/'))});")
                    r = await send("Page.captureScreenshot", format="png")
                    path = os.path.join(OUT, f"{name}-{lang}.webp")
                    Image.open(io.BytesIO(base64.b64decode(r["data"]))).convert("RGB").save(path, "WEBP", quality=86, method=6)
                    print(f"  {os.path.relpath(path, ROOT)}  {os.path.getsize(path) // 1024} KB")
    finally:
        proc.terminate()
        proc.wait(timeout=10)
        shutil.rmtree(profile, ignore_errors=True)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--lang", choices=["zh", "en"], nargs="*", default=["zh", "en"])
    ap.add_argument("--pages", nargs="*", default=[], help="lattice lattice3d run results learn (default: all)")
    ap.add_argument("--browser", help="Chrome / Edge executable")
    args = ap.parse_args(argv)
    browser = args.browser or next((b for b in BROWSERS if os.path.isfile(b)), None)
    if not browser:
        ap.error("no Chrome or Edge found; pass --browser")
    os.makedirs(OUT, exist_ok=True)
    work = tempfile.mkdtemp(prefix="avas-readme-")
    server = None
    try:
        print("demo project and run ...")
        project = demo_project(work)
        settings = os.path.join(work, "gui.json")
        with open(settings, "w", encoding="utf-8") as fh:
            json.dump({"ui/theme": "light", "ui/motion": "full"}, fh)
        port = free_port()
        server = subprocess.Popen([sys.executable, "-m", "avas", "serve", "--port", str(port), "--token", TOKEN,
                                   "--settings", settings], cwd=ROOT)
        url = f"http://127.0.0.1:{port}/index.html?host=browser&token={TOKEN}"
        for _ in range(100):
            try:
                urllib.request.urlopen(url, timeout=1)
                break
            except Exception:  # noqa: BLE001 - the server is still starting
                time.sleep(0.2)
        # open the project before the page loads: the page reads the open project when it starts
        req = urllib.request.Request(f"http://127.0.0.1:{port}/api/rpc", method="POST",
                                     data=json.dumps({"method": "project.open", "params": {"path": project}}).encode(),
                                     headers={"Content-Type": "application/json", "X-AVAS-Token": TOKEN})
        if not json.load(urllib.request.urlopen(req, timeout=30)).get("ok"):
            raise RuntimeError("the demo project did not open")
        print("screenshots ...")
        asyncio.run(capture(browser, url, project, args.lang, set(args.pages)))
    finally:
        if server:
            server.terminate()
            server.wait(timeout=10)
        shutil.rmtree(work, ignore_errors=True)


if __name__ == "__main__":
    main()
