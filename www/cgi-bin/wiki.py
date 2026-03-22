import requests, sys, json, os

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

def get_summary(keyword):
    headers = {
        "User-Agent": "MyApp/1.0 (contact@example.com)"
    }

    params = {
        "action": "query",
        "list": "search",
        "srsearch": keyword,
        "srlimit": 1,
        "format": "json"
    }
    response = requests.get(
        "https://en.wikipedia.org/w/api.php",
        params=params, headers=headers
    )
    if not response.ok:
        return None

    results = response.json()["query"]["search"]
    if not results:
        return None

    title = results[0]["title"]

    response = requests.get(
        f"https://en.wikipedia.org/api/rest_v1/page/summary/{title}",
        headers=headers
    )
    if not response.ok:
        return None

    data = response.json()
    return {
        "title": data["title"],
        "summary": data["extract"]
    }

response = ""
result = get_summary(body)
if result:
    title = json.dumps(result["title"])
    summary = json.dumps(result["summary"])
    response = title + "\n" + summary + "\n"
else:
    response = "Nothing found."


sys.stdout.write(f"Status: 200 OK\r\n")
sys.stdout.write(f"Content-Type: text/html\r\n")
sys.stdout.write(f"Content-Length: {len(response)}\r\n")
sys.stdout.write(f"\r\n")
sys.stdout.write(response)
