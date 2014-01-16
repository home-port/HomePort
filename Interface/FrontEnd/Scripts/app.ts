/// <reference path="jquery.d.ts" />
/// <reference path="jqueryui.d.ts" />



//extend the Window interface.
interface Window {
    ScenarioDOM: ScenarioDOM;
    CanvasDOM: DeviceCanvas;
}

window.onload = () => {
    //DEBUG

    var whenArray = new Array<ScenarioEvent>();
    var ifArray = new Array<ScenarioCondition>();
    var thenArray = new Array<ScenarioAction>();
    for (var i = 0; i < 3; i++) {
        whenArray.push(new ScenarioEvent("whenDevice_" + i, "<5", 2 - i, 5, "foobar"));
        ifArray.push(new ScenarioCondition("ifDevice_" + i, "<5"));
        thenArray.push(new ScenarioAction("thenDevice_" + i, "<5", 2 - i));
    }
    var scenario = new Scenario(whenArray, ifArray, thenArray);

    this.ScenarioDOM = new ScenarioDOM(scenario, document.getElementById("whenWrap"), document.getElementById("ifWrap"), document.getElementById("thenWrap"));
    this.CanvasDOM = new DeviceCanvas(document.getElementById("canvas"));
}