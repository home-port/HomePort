/// <reference path="../jquery.d.ts" />
/// <reference path="../jqueryui.d.ts" />

class Block {

    private _model: ScenarioBlock;
    public get Model() { return this._model; }
    public set Model(val) { this._model = val; }


    private _id;
    public get Id() { return this._id; }
    public set Id(val) { this._id = val; }

    private _DOMServiceType;
    public get ServiceType() { return this._DOMServiceType; }
    public set ServiceType(val) { this._DOMServiceType = val; }

    private _timelineType;
    public get TimelineType() { return this._timelineType; }
    public set TimelineType(val) {
        this._timelineType = val;
        this.DOM = this.GetDOM();
    }

    private _dom: HTMLElement;
    public get DOM() {
        return this._dom;
    }
    public set DOM(val) { this._dom = val; }

    public SetContent(DOMWrapper : HTMLElement) {
        throw "Not implemented in sub class";
    }
    public SetCustomContent(DOMWrapper: HTMLElement, custom: HTMLElement)
    {
        DOMWrapper.innerHTML = "";
        DOMWrapper.appendChild(custom);
    }


    //reset the left and top css property after a drag.
    public ResetDragOffset() {
        this.DOM.style.removeProperty('left');
        this.DOM.style.removeProperty('top');
    }

    public CreateContent(DOMWrapper: HTMLElement) : HTMLElement {
        throw "Not implemented in sub class";
    }


    public ReplaceDeviceText(deviceName: string) {
        throw "Not implemented"; //TODO
    }

    public ReplaceServiceText(serviceName: string, value: string) {
        throw "Not implemented"; //TODO
    }

    public ChangeParentInfo(newParent: string) {
        this.DOM.dataset['timelineType'] = newParent;
    }

    constructor(model, id: string) {
        this.Model = model;
        this.Id = id;
    }

    public ResizeHeight(height: number) {
        throw "Not implemented in sub class";
    }

    public ResizeWidht(width: number) {
        throw "Not implemented in sub class";
    }

    public GetDOM(): any {
        throw "Not implemented in sub class";

    }
}

class StartBlock extends Block {

    constructor(id: string) {
        super(null, id);
        this.DOM = this.GetDOM();
    }

    public GetDOM() {
        return BlockView.GetStartView(this.Id);
    }
}

class DeviceBlock extends Block {
    constructor(model: ScenarioBlock, id: string, blockType: string) {
        super(model, id);

        this.ServiceType = ServiceTypeEnum.Actuator; //DEBUG. TODO: get this value from the model->service->attribute?.
        this.TimelineType = blockType;
    }

    

    public SetContent(DOMWrapper: HTMLElement) {
        this.SetCustomContent(DOMWrapper, this.CreateContent());
    }

    public CreateContent() : HTMLElement {
        return BlockView.GetBlockContentView(this.Id, this.Model.DeviceID, this.Model.ServiceID, this.Model.Value);
    }

    public GetDOM(): HTMLElement {
        var output;


        //JqueryUI doesnt modify left, top correctly if relative position is modified by the DOM while dragging. Let's work around that :).
        function onDrag(serviceType) {
            return (event: JQueryUI.DraggableEvent, ui: JQueryUI.DraggableEventUIParams) => {
                var blockDOM = ui.helper[0];
                //Figure out the current position
                var oldPos = blockDOM.offsetLeft;
                blockDOM.style.position = "absolute";
                blockDOM.style.left = (blockDOM.offsetLeft + blockDOM.style.left).toString();
                blockDOM.classList.remove("appended");
                blockDOM.style.zIndex = "10";
                
                window.ScenarioDOM.InsertDropSpaces(serviceType);
            }
        }

        function onDrop() {
            return (event: JQueryUI.DraggableEvent, ui: JQueryUI.DraggableEventUIParams)  => {
                window.ScenarioDOM.RemoveDropSpaces();
                var blockDOM = ui.helper[0];
                blockDOM.classList.add("appended");
                blockDOM.style.position = "relative";
                blockDOM.style.removeProperty("zIndex");

            }
        }

        switch (this.TimelineType) {
            case BlockTypeEnum.When:
                output = BlockView.GetTimedBlockView(this.Id, this.TimelineType, this.CreateContent(), onDrag(this.ServiceType), (event, ui) => { }, onDrop() );
                break;
            case BlockTypeEnum.If:
                break;
            case BlockTypeEnum.Then:
                output = BlockView.GetTimedBlockView(this.Id, this.TimelineType, this.CreateContent(), onDrag(this.ServiceType), (event, ui) => { }, onDrop() );
                break;
            case BlockTypeEnum.None:
                break;
            default: 
                throw "blocktype not set on id:" + this.Id;
                break;
        }

        this.DOM = output;
        return output;
    }
}


class DropSpaceTimedBlock extends Block {

    private _index: number;
    public get Index() { return this._index; }
    public set Index(val) { this._index = val; }

    private _callbacks: Array<Function>;
    public get Callbacks() { return this._callbacks; }
    public set Callbacks(val) { this._callbacks = val;}

    constructor(id: string, index, callbacks : Array<Function>, blockType: string) {
        super(null, id);
        this.Index = index;
        this.Callbacks = callbacks;
        this.TimelineType = blockType;
    }

    public GetDOM() {
        return BlockView.GetTimedDropView(this.Id, this.TimelineType, this.Index, this.Callbacks);
    }
}
 