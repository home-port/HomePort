/// <reference path="../jquery.d.ts" />
/// <reference path="../jqueryui.d.ts" />

class DeviceCanvas {
    private _dom: HTMLElement;
    public get DOM() { return this._dom; }
    public set DOM(val) { this._dom = val; }


    constructor(dom: HTMLElement) {
        this.DOM = dom;

        //Make the canvas droppable
        $(this.DOM).droppable({});


        //a few sample buttons
        var event = new ScenarioEvent("STATICSERVICEID", "<X", "-1", "4", "FOO");

        var temp1 = new DeviceBlock(event, 'test', BlockTypeEnum.When);
        var content = document.createElement('div');
        content.textContent = "Drag this one";
        temp1.SetContent(content);
        this.DOM.appendChild(temp1.GetDOM());

        $("#" + temp1.Id).draggable({
            handle: "#" + temp1.Id + "_handle",
            containment: "#wrapper",
            revert: "invalid",
            start: function (event, ui: HTMLElement) {

                console.log("START: drag of element id: " + ui.id + ".");
            },
            stop: function (event, ui) {
                console.log("STOP: drag of element id: " + ui.id + ".");
            }
        });
    }
} 