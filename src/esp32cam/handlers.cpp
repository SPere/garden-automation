#include "AsyncCam.hpp"
#include <StreamString.h>

static const char FRONTPAGE[] = R"EOT(
<!doctype html>
<title>esp32cam AsyncCam example</title>
<body>
<h1>esp32cam AsyncCam example</h1>
<form id="update"><p>
<select name="resolution">%resolution%</select>
%brightness%
%contrast%
%saturation%
<select name="gain" title="gain">%gain%</select>
<select name="lightMode" title="light mode">%lightMode%</select>
<select name="specialEffect" title="special effect">%specialEffect%</select>
%hmirror%
%vflip%
%rawGma%
%lensCorrection%
<input type="submit" value="update">
</p></form>
<p id="controls">
<button data-res="cam.jpg">still JPEG</button>
<button data-res="cam.mjpeg">MJPEG</button>
<button data-res="continuous.mjpeg">MJPEG-continuous</button>
<button data-res="">hide</button>
</p>
<div id="display"></div>
<footer>Powered by <a href="https://esp32cam.yoursunny.dev/">esp32cam</a></footer>
<script type="module">
async function fetchText(uri, init) {
  const response = await fetch(uri, init);
  if (!response.ok) {
    throw new Error(await response.text());
  }
  return (await response.text()).trim().replaceAll("\r\n", "\n");
}

const $update = document.querySelector("#update");
const $display = document.querySelector("#display");
$update.addEventListener("submit", async (evt) => {
  evt.preventDefault();
  try {
    await fetchText("/update.cgi", {
      method: "POST",
      body: new FormData($update),
    });
  } catch (err) {
    $display.textContent = err.toString();
  }
});
$update.reset();

for (const $ctrl of document.querySelectorAll("#controls button")) {
  $ctrl.addEventListener("click", (evt) => {
    evt.preventDefault();

    const $img = $display.querySelector("img");
    if ($img) {
      $img.src = "";
    }

    const resource = evt.target.getAttribute("data-res");
    $display.innerHTML = resource && `<img src="/${resource}?_=${Math.random()}" alt="${resource}">`;
  });
}
</script>
)EOT";

static String
rewriteFrontpage(const esp32cam::Settings& s, const String& var) {
  StreamString b;

  if (var == "resolution") {
    for (const auto& r : esp32cam::Camera.listResolutions()) {
      b.printf("<option value=\"%d\"%s>", r.as<int>(),
               r > initialResolution ? " disabled"
               : r == s.resolution   ? " selected"
                                     : "");
      b.print(r);
      b.print("</option>");
    }
  } else if (var == "gain") {
#define SHOW_GAIN(val, dsp)                                                                        \
  b.printf("<option value=\"%d\"%s>%dx</option>", val, s.gain == val ? " selected" : "", dsp)
    b.printf("<optgroup label=\"AGC=0\">");
    for (int i = 1; i <= 31; ++i) {
      SHOW_GAIN(i, i);
    }
    b.printf("</optgroup>");
    b.printf("<optgroup label=\"AGC=1\">");
    for (int i = 2; i <= 128; i <<= 1) {
      SHOW_GAIN(-i, i);
    }
    b.printf("</optgroup>");
#undef SHOW_GAIN
  } else if (var == "lightMode") {
#define SHOW_LM(MODE, symbol)                                                                      \
  b.printf("<option value=\"%d\" title=\"%s\"%s>%s</option>",                                      \
           static_cast<int>(esp32cam::LightMode::MODE), #MODE,                                     \
           s.lightMode == esp32cam::LightMode::MODE ? " selected" : "", symbol)
    SHOW_LM(NONE, "&#x1F6AB;");
    SHOW_LM(AUTO, "&#x2B55;");
    SHOW_LM(SUNNY, "&#x2600;&#xFE0F;");
    SHOW_LM(CLOUDY, "&#x2601;&#xFE0F;");
    SHOW_LM(OFFICE, "&#x1F3E2;");
    SHOW_LM(HOME, "&#x1F3E0;");
#undef SHOW_LM
  } else if (var == "specialEffect") {
#define SHOW_SE(MODE, symbol)                                                                      \
  b.printf("<option value=\"%d\" title=\"%s\"%s>%s</option>",                                      \
           static_cast<int>(esp32cam::SpecialEffect::MODE), #MODE,                                 \
           s.specialEffect == esp32cam::SpecialEffect::MODE ? " selected" : "", symbol)
    SHOW_SE(NONE, "&#x1F6AB;");
    SHOW_SE(NEGATIVE, "&#x2B1C;");
    SHOW_SE(BLACKWHITE, "&#x2B1B;");
    SHOW_SE(REDDISH, "&#x1F7E5;");
    SHOW_SE(GREENISH, "&#x1F7E9;");
    SHOW_SE(BLUISH, "&#x1F7E6;");
    SHOW_SE(ANTIQUE, "&#x1F5BC;&#xFE0F;");
#undef SHOW_SE
  }

#define SHOW_INT(MEM, MIN, MAX)                                                                    \
  else if (var == #MEM) {                                                                          \
    b.printf("<label>" #MEM "=<input type=\"number\" name=\"" #MEM                                 \
             "\" value=\"%d\" min=\"%d\" max=\"%d\"></label>",                                     \
             s.MEM, MIN, MAX);                                                                     \
  }

#define SHOW_BOOL(MEM)                                                                             \
  else if (var == #MEM) {                                                                          \
    b.printf("<label><input type=\"checkbox\" name=\"" #MEM "\" value=\"1\"%s>" #MEM "</label>",   \
             s.MEM ? " checked" : "");                                                             \
  }

  SHOW_INT(brightness, -2, 2)
  SHOW_INT(contrast, -2, 2)
  SHOW_INT(saturation, -2, 2)
  SHOW_BOOL(hmirror)
  SHOW_BOOL(vflip)
  SHOW_BOOL(rawGma)
  SHOW_BOOL(lensCorrection)
#undef SHOW_INT
#undef SHOW_BOOL
  return b;
}

static void
handleFrontpage(AsyncWebServerRequest* req) {
  Serial.printf("[HTTP] [%s] GET /\n", req->client()->remoteIP().toString().c_str());
  
  uint32_t startTime = millis();
  static const size_t contentLength = strlen(FRONTPAGE);
  auto settings = esp32cam::Camera.status();
  
  AsyncWebServerResponse *response = new AsyncProgmemResponse(
      200, 
      "text/html", 
      reinterpret_cast<const uint8_t*>(FRONTPAGE), 
      contentLength, 
      [=](const String& var) { return rewriteFrontpage(settings, var); }
  );

  req->send(response);
  Serial.printf("[HTTP] Sent frontpage template in %d ms\n", (millis() - startTime));
}

static void
handleUpdate(AsyncWebServerRequest* req) {
  Serial.printf("[HTTP] [%s] POST /update.cgi\n", req->client()->remoteIP().toString().c_str());
  
  bool ok = esp32cam::Camera.update([=](esp32cam::Settings& s) {
#define SAVE_BOOL(MEM) s.MEM = req->hasArg(#MEM)
#define SAVE_INT(MEM) s.MEM = decltype(s.MEM)(req->arg(#MEM).toInt())
    SAVE_INT(resolution);
    SAVE_INT(brightness);
    SAVE_INT(contrast);
    SAVE_INT(saturation);
    SAVE_INT(gain);
    SAVE_INT(lightMode);
    SAVE_INT(specialEffect);
    SAVE_BOOL(hmirror);
    SAVE_BOOL(vflip);
    SAVE_BOOL(rawGma);
    SAVE_BOOL(lensCorrection);
#undef SAVE_BOOL
#undef SAVE_INT
  });

  if (!ok) {
    Serial.println("[HTTP] ERROR: Camera settings update failed");
    req->send(500, "text/plain", "update settings error\n");
    return;
  }
  
  Serial.println("[HTTP] Camera settings successfully updated");
  req->send(204);
}

void addRequestHandlers() {
  server.on("/robots.txt", HTTP_GET, [](AsyncWebServerRequest* req) {
    Serial.printf("[HTTP] [%s] GET /robots.txt\n", req->client()->remoteIP().toString().c_str());
    req->send(200, "text/plain", "User-Agent: *\nDisallow: /\n");
  });

  server.on("/", HTTP_GET, handleFrontpage);
  server.on("/update.cgi", HTTP_POST, handleUpdate);

  // STABLE STILL CAPTURE ROUTE WITH METRIC LOGGING
  server.on("/cam.jpg", HTTP_GET, [](AsyncWebServerRequest *req) {
    Serial.printf("[HTTP] [%s] GET /cam.jpg (Requesting capture...)\n", req->client()->remoteIP().toString().c_str());
    
    uint32_t captureStart = millis();
    auto frame = esp32cam::Camera.capture();
    uint32_t captureTime = millis() - captureStart;

    if (!frame) {
      Serial.println("[HTTP] ERROR: Camera capture failed completely!");
      req->send(500, "text/plain", "Camera capture failed");
      return;
    }
    
    Serial.printf("[HTTP] Capture success: %d bytes in %d ms\n", frame->size(), captureTime);
    
    AsyncWebServerResponse *response = req->beginResponse_P(
      200, 
      "image/jpeg", 
      frame->data(), 
      frame->size()
    );
    response->addHeader("Content-Disposition", "inline; filename=capture.jpg");
    req->send(response);
  });
 // STABLE MJPEG STREAMING ROUTE WITH FIXES
  server.on("/cam.mjpeg", HTTP_GET, [](AsyncWebServerRequest *req) {
    Serial.printf("[HTTP] [%s] GET /cam.mjpeg (Starting stream...)\n", req->client()->remoteIP().toString().c_str());
    
    AsyncWebServerResponse *response = req->beginChunkedResponse("multipart/x-mixed-replace;boundary=123456789000000000000987654321", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
      auto frame = esp32cam::Camera.capture();
      if (!frame) {
        return 0;
      }
      
      char header[128]; // <-- FIXED: Correct array allocation size
      snprintf(header, sizeof(header), "\r\n--123456789000000000000987654321\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", frame->size());
      size_t headLen = strlen(header);
      
      if (headLen + frame->size() > maxLen) {
        return 0;
      }
      
      memcpy(buffer, header, headLen);
      memcpy(buffer + headLen, frame->data(), frame->size());
      return headLen + frame->size();
    });
    req->send(response);
  });
}
