const char html_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
.slidecontainer {
  width: 100%;
}

.slider {
  -webkit-appearance: none;
  width: 100%;
  height: 15px;
  border-radius: 5px;
  background: #d3d3d3;
  outline: none;
  opacity: 0.7;
  -webkit-transition: .0s;
  transition: opacity .2s;
}

.slider:hover {
  opacity: 1;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 25px;
  height: 25px;
  border-radius: 50%;
  background: #326C88;
  cursor: pointer;
}

.slider::-moz-range-thumb {
  width: 25px;
  height: 25px;
  border-radius: 50%;
  background: #326C88;
  cursor: pointer;
}

/* Rounded switch */

.switch {
  position: relative;
  display: inline-block;
  width: 60px;
  height: 34px;
}

.switch input { 
  opacity: 0;
  width: 0;
  height: 0;
}

.slider1 {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
}

.slider1:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .2s;
  transition: .2s;
}

input:checked + .slider1 {
  background-color: #326C88;
}

input:focus + .slider1 {
  box-shadow: 0 0 1px #326C88;
}

input:checked + .slider1:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}

.slider1.round {
  border-radius: 34px;
}

.slider1.round:before {
  border-radius: 50%;
}

</style>
</head>
<body>

<h2>AutoMATE HTTP Server Demo</h2>

<div class="slidecontainer">
  <p>IO1 LED PWM output: <span id="slider_label1"></span></p>
  <input type="range" min="0" max="1000" value="0" class="slider" id="slider_val1">
  
  <p>IO2 0-10V output: <span id="slider_label2"></span></p>
  <input type="range" min="0" max="10000" value="0" class="slider" id="slider_val2">
  
  <p>IO3 output voltage: <span id="slider_label3"></span></p>
  <input type="range" min="0" max="24000" value="0" class="slider" id="slider_val3">
  
  <p>IO3 output frequency: <span id="slider_label4"></span></p>
  <input type="range" min="0" max="5000" value="0" class="slider" id="slider_val4">
  
  <p>IO3 output duty-cycle: <span id="slider_label5"></span></p>
  <input type="range" min="0" max="1000" value="0" class="slider" id="slider_val5">

  <p>Relay 1: <span id="relay_state1">OFF</span></p> 
  <label class="switch">
    <input type="checkbox" onchange="state_change(this, 1)">
    <span class="slider1 round"></span>
  </label>
  
  <p>Relay 2: <span id="relay_state2">OFF</span></p> 
  <label class="switch">
    <input type="checkbox" onchange="state_change(this, 2)">
    <span class="slider1 round"></span>
  </label>
  
  <p>Relay 3: <span id="relay_state3">OFF</span></p> 
  <label class="switch">
    <input type="checkbox" onchange="state_change(this, 3)">
    <span class="slider1 round"></span>
  </label>
  
  <p>Relay 4: <span id="relay_state4">OFF</span></p> 
  <label class="switch">
    <input type="checkbox" onchange="state_change(this, 4)">
    <span class="slider1 round"></span>
  </label>
  
  <p>MCU temperature (C): <span id="temp_value">0</span></p>
  <p>3V3 rail voltage (mV): <span id="voltage3v3_value">0</span></p>
  <p>5V rail voltage (mV): <span id="voltage5v0_value">0</span></p>
  <p>RTC time: <span id="rtc_value">0:0:0</span></p>
</div>

<script src="https://canvasjs.com/assets/script/canvasjs.min.js"></script>


<script>
document.getElementById("slider_val1").onchange = function() {
  document.getElementById("slider_label1").innerHTML = this.value;
  slider_change(this.value, 1);
}

document.getElementById("slider_val2").onchange = function() {
  document.getElementById("slider_label2").innerHTML = this.value;
  slider_change(this.value, 2);
}

document.getElementById("slider_val3").onchange = function() {
  document.getElementById("slider_label3").innerHTML = this.value;
  slider_change(this.value, 3);
}

document.getElementById("slider_val4").onchange = function() {
  document.getElementById("slider_label4").innerHTML = this.value;
  slider_change(this.value, 4);
}

document.getElementById("slider_val5").onchange = function() {
  document.getElementById("slider_label5").innerHTML = this.value;
  slider_change(this.value, 5);
}

setInterval(function() 
{
  getData();
},2000);

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temp_value").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setTemp", true);
  xhttp.send();
  
  xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("voltage3v3_value").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setVoltage3V3", true);
  xhttp.send();
  
  xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("voltage5v0_value").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setVoltage5V0", true);
  xhttp.send();
  
  xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("rtc_value").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setRTC", true);
  xhttp.send();
}
</script>

<script>
function slider_change(val, index) {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "setSlider?slider_val" + index.toString() + "=" + val, true);
  xhttp.send();
}
</script>

<script>
function state_change(element, index) {
  var xhttp = new XMLHttpRequest();
  var element_id = "relay_state" + index.toString();

  if (element.checked){
    xhttp.open("GET", "setButton?button_state" + index.toString() + "=1", true);
    document.getElementById(element_id).innerHTML = "ON";
  } else if (!element.checked){
    xhttp.open("GET", "setButton?button_state" + index.toString() + "=0", true);
    document.getElementById(element_id).innerHTML = "OFF";
  }
  xhttp.send();
}
</script>

</body>

</html>
)====="; 
