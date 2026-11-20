# نبض - مراقب النبض والأوكسجين

ESP32 + MAX30102 + LCD I2C، والموقع يستلم القراءات عبر MQTT.

## الملفات
- `index.html`: الموقع كاملاً (ملف واحد)
- `netlify.toml`: إعداد Netlify
- `esp32_nabd/esp32_nabd.ino`: كود الجهاز

## 1) Visual Studio Code
1. افتح المجلد: File > Open Folder.
2. ثبّت إضافة **Live Server** وانقر Go Live لتجربة الموقع محلياً.
3. للجهاز: ثبّت إضافة **PlatformIO** أو افتح الـ `.ino` بـ Arduino IDE.

## 2) GitHub
```bash
git init
git add .
git commit -m "Nabd project"
git branch -M main
git remote add origin https://github.com/USERNAME/nabd.git
git push -u origin main
```
ثم من المستودع: Settings > Pages > Branch: main > Save. يظهر الرابط بعد دقيقة.

## 3) Netlify
- الأسهل: ادخل app.netlify.com/drop واسحب مجلد المشروع.
- أو: Add new site > Import from Git > اختر المستودع > Publish directory: `.`

## 4) ESP32
1. ثبّت المكتبات: SparkFun MAX3010x، LiquidCrystal I2C، PubSubClient.
2. عدّل WIFI_SSID وWIFI_PASS وTOPIC.
3. اللوحة: ESP32 Dev Module، ثم Upload.
4. بالموقع: الربط > نفس القناة > اتصال.

القناة (TOPIC) لازم تتطابق حرفياً بين الموقع والكود. الخادم العام HiveMQ مفتوح للجميع، فلا ترسل عليه بيانات مرضى حقيقية.
