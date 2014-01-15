
class BlockView {
    public static GetTimedBlockView(id: string, content : HTMLElement, onDragCallback : Function, onDropCallback : Function) {
        //Container element
        var output = document.createElement('div');
        output.id = id;
        output.classList.add('block');
        output.classList.add('timedBlock');
        output.classList.add('blockSpacing');
        output.classList.add('appended');

        //start of arrow 
        output.appendChild(UIHelper.CreateStartArrow(id + "_startArrow", new Array(), new Array("timedBlock")));

        //dynamic content element
        var contentWrap = document.createElement('div');
        contentWrap.id = id + "_content";
        contentWrap.classList.add("contentBackground");
        contentWrap.classList.add("blockContent");
        contentWrap.appendChild(content);
        output.appendChild(contentWrap);

        //drag area
        var drag = document.createElement('div')
        drag.id = id + "_handle";
        drag.classList.add('dragHere'); //class used for jqueryui selector
        output.appendChild(drag);

        //end of arrow (pointy end)
        output.appendChild(UIHelper.CreateEndArrow(id + "_endArrow", new Array("blockspacing"), new Array("timedBlock")));


        //Hook up events
        $(output).draggable({
            handle: "#" + id + "_handle",
            containment: "#wrapper",
            revert: "invalid",
            start: function (event, ui: HTMLElement) {
                onDragCallback(event, ui);
                console.log("START: drag of element id: " + ui.id + ".");
            },
            stop: function (event, ui) {
                onDropCallback(event, ui);
                console.log("STOP: drag of element id: " + ui.id + ".");
            }
        });

        return output;
    }

    public static GetBlockContentView(id, deviceID, serviceID, value) {
        var output = document.createElement('div');
        var device = document.createElement('div');
        device.id = id + "_contentDeviceID";
        device.textContent = deviceID;

        var service = document.createElement('Div');
        service.id = id + "_contentServiceID";
        service.textContent = serviceID + value;

        output.appendChild(device);
        output.appendChild(service);
        return output;
    } 


    public static GetTimedDropView(id: string) {
        //Container element
        var output = document.createElement('div');
        output.id = id;
        output.classList.add('block');
        output.classList.add('timedBlock');
        output.classList.add('blockSpacing');
        output.classList.add('dropArrow');
        output.classList.add('appended');

        //start of arrow 
        output.appendChild(UIHelper.CreateStartArrow(id+ "_startArrow", new Array(), new Array("timedBlock", "dropspace")));

        //filler
        var middle = document.createElement("div");
        middle.textContent = "Drop here!"
        middle.classList.add("blockContent");
        middle.classList.add("dropSpaceBackground");
        output.appendChild(middle);

        //drop area

        var drop = document.createElement('div')
        drop.id = id +"_dropArea";
        drop.classList.add('dropHere'); //class used for jqueryui selector
        middle.appendChild(drop);


        //end of arrow (pointy end)
        output.appendChild(UIHelper.CreateEndArrow(id + "_endArrow", new Array("blockspacing"), new Array("timedBlock", "dropspace")));
        return output;
    }
}

var BlockTypeEnum = {
    When: 'When',
    If: 'If',
    Then: 'Then',
    None: 'None'
}
