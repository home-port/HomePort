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
    public get DOMServiceType() { return this._DOMServiceType; }
    public set DOMServiceType(val) { this._DOMServiceType = val; }

    private _blockType;
    public get BlockType() { return this._blockType; }
    public set BlockType(val) {
        this._blockType = val;
        this.DOM = this.GetDOM();
    }

    private _dom: Element;
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

    public CreateContent(DOMWrapper: HTMLElement) : HTMLElement {
        throw "Not implemented in sub class";
    }


    public ReplaceDeviceText(deviceName: string) {
        throw "Not implemented"; //TODO
    }

    public ReplaceServiceText(serviceName: string, value: string) {
        throw "Not implemented"; //TODO
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
        return UIHelper.CreateEndArrow(this.Id + "_endArrow", new Array(), new Array("startTriangle", "block"));
    }
}

class DeviceBlock extends Block {
    constructor(model: ScenarioBlock, id: string, blockType: string) {
        super(model, id);

        this.DOMServiceType = ServiceTypeEnum.Actuator; //DEBUG. TODO: get this value from the model->service->attribute?.
        this.BlockType = blockType;
    }



    public SetContent(DOMWrapper: HTMLElement) {
        this.SetCustomContent(DOMWrapper, this.CreateContent());
    }

    public CreateContent() : HTMLElement {
        return BlockView.GetBlockContentView(this.Id, this.Model.DeviceID, this.Model.ServiceID, this.Model.Value);
    }

    public GetDOM(): HTMLElement {
        switch (this.BlockType) {
            case BlockTypeEnum.When:
                return BlockView.GetTimedBlockView(this.Id, this.CreateContent());
                break;
            case BlockTypeEnum.If:
                break;
            case BlockTypeEnum.Then:
                return BlockView.GetTimedBlockView(this.Id, this.CreateContent());
                break;
            case BlockTypeEnum.None:
                break;
            default: 
                throw "blocktype not set on id:" + this.Id;
                break;
        }
    }
}


class DropSpaceTimedBlock extends Block {
    constructor(id: string) {
        super(null, id);
    }

    public GetDOM() {
        return BlockView.GetTimedDropView(this.Id);
    }

}
 