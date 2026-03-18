#!/usr/bin/env python3
import os, sys, html

CGI_VARS = [
    "CONTENT_LENGTH",
    "CONTENT_TYPE",
    "GATEWAY_INTERFACE",
    "QUERY_STRING",
    "REMOTE_ADDR",
    "REMOTE_HOST",
    "REQUEST_METHOD",
    "SCRIPT_NAME",
    "SERVER_NAME",
    "SERVER_PORT",
    "SERVER_PROTOCOL",
    "SERVER_SOFTWARE",
]

length = int(os.environ.get("CONTENT_LENGTH", 0))
body = ""
if length > 0:
    body = sys.stdin.read(length)

rows = ""
for var in CGI_VARS:
    val = html.escape(os.environ.get(var, "(not set)"))
    rows += f"<tr><td><code>{var}</code></td><td>{val}</td></tr>\n"

escaped_body = html.escape(body) if body else "<em>empty</em>"

page = f"""\

<html>
<head><title>CGI Debug2</title></head>
<style>
    body {{ font-family: sans-serif; margin: 40px; }}
    .dark {{ background: #222; color: white; }}
    button {{ padding: 10px 20px; cursor: pointer; }}
</style>
<body>
<button onclick="toggle()">Toggle Theme</button>
<h1>CGI Environment</h1>
<table border="1" cellpadding="4" cellspacing="0">
<tr><th>Variable</th><th>Value</th></tr>
{rows}
</table>
<h2>Request Body</h2>
<pre>{escaped_body}</pre>
<p><strong>Body length:</strong> {len(body)} bytes</p>
<script>
function toggle() {{
    document.body.classList.toggle('dark');
}}
</script>
</body>
</html>
"""

sys.stdout.write(f"Status: 200 OK\r\n")
sys.stdout.write(f"Content-Type: text/html\r\n")
sys.stdout.write(f"Content-Length: {len(page)}\r\n")
sys.stdout.write(f"\r\n")
sys.stdout.write(page)