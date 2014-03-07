/// <reference path="../jquery.d.ts" />
/// <reference path="../jqueryui.d.ts" />

class ScenarioDOM {
    private _model: Scenario;
    public get Model() { return this._model; }
    public set Model(val) { this._model = val;}

    private _whenTimeline: WhenScenarioTimeline;
    public get When() { return this._whenTimeline; }
    public set When(val) { this._whenTimeline = val; }

    private _ifTimeline: ScenarioTimeline;
    public get If() { return this._ifTimeline }
    public set If(val) { this._ifTimeline = val; }

    private _thenTimeline: ScenarioTimeline;
    public get Then() { return this._thenTimeline }
    public set Then(val) { this._thenTimeline = val; }

    public InsertDropSpaces(serviceType: string) {
        this.When.InsertDropSpaces(() => { });
        this.If.InsertDropSpaces(() => { });

        //only add dropspaces to timelines which support this type of service (no sensor in "then")
        if (serviceType === ServiceTypeEnum.Actuator) {
            this.Then.InsertDropSpaces(() => { });
        }
    }

    public RemoveDropSpaces() {
        this.When.RemoveDropSpaces();
        this.If.RemoveDropSpaces();
        this.Then.RemoveDropSpaces();
    }

    public UpdateModel() {
        throw "Not Implemented";
    }

    public UpdateDOM() {
        throw "Not Implemented";
    }

    public Populate() {
        throw "Not Implemented";
    }

    constructor(scenario: Scenario , whenWrapper: HTMLElement, ifWrapper: HTMLElement, thenWrapper: HTMLElement) {
        //Initialize scenario
        this.Model = scenario;
        this.When = new WhenScenarioTimeline("When", scenario.When, whenWrapper, BlockTypeEnum.When);
        this.If = new ScenarioTimeline("If", scenario.If, ifWrapper, BlockTypeEnum.If); //TODO IMPLEMENT in subclass
        this.Then = new ThenScenarioTimeline("Then", scenario.Then, thenWrapper, BlockTypeEnum.Then); 


        //populate 
        this.When.Render();
        this.Then.Render();



    }

}

class ScenarioTimeline {

    private _id: string;
    public get Id() { return this._id; }
    public set Id(val) { this._id = val;}

    private _models: Array<ScenarioBlock>;
    public get Models() { return this._models; }
    public set Models(val) { this._models = val; }

    private wrapper: HTMLElement;
    public get DOMWrapper() { return this.wrapper; }
    public set DOMWrapper(val) { this.wrapper = val; }

    private _blocks: Array<Block>;
    public get Blocks() { return this._blocks; }
    public set Blocks(val) { this._blocks = val; }

    private _timelineType: string;
    public get TimelineType() { return this._timelineType; }
    public set TimelineType(val) { this._timelineType = val; }

    //get block by ID
    public GetBlockByID(id: string) : Block {
        for (var i = 0; i < this.Blocks.length; i++)
        {
            if (this.Blocks[i].Id === id) {
                return this.Blocks[i];
            }
        }
        
    }

    //insert specific block in this.block and DOM
    public InsertBlockAfterIndex(index: number, newBlock: Block) {
        if (index >= 0) {
            var rest = this.Blocks.splice(index + 1);
            this.Blocks.push(newBlock);
            this.Blocks = this.Blocks.concat(rest);
            //Move the DOM.
            $(newBlock.DOM).insertAfter(this.Blocks[index].DOM);
            //change the parentType of the DOM
            newBlock.ChangeParentInfo(this.TimelineType)
            
        }

        if (index < 0) {
            this.Blocks.push(newBlock);
            this.DOMWrapper.appendChild(newBlock.DOM);
        }
        //reset the blocks drag offset
        newBlock.ResetDragOffset();

    }
    //remove specific index from this.blocks and DOM
    public RemoveBlockIndex(index: number) {
        var rest = this.Blocks.splice(index + 1);
        var remove = this.Blocks.pop();
        $(remove).remove();
        this.Blocks = this.Blocks.concat(rest);
    }

    

    //insert block after specific block in this.blocks and DOM
    public InsertBlockAfter(after: Block, newBlock: Block) {
        for (var i = 0; i < this.Blocks.length; i++) {
            if (this.Blocks[i] === after) {
                var rest = this.Blocks.splice(i + 1);
                this.Blocks.push(newBlock);
                this._blocks.concat(rest);
                //Move the DOM.
                $(newBlock.DOM).insertAfter(after.DOM);
                newBlock.ResetDragOffset();

                break;
            }
        }
    }
    //remove specific block from DOM and this.Blocks
    public RemoveBlock(block: Block) {
        var block = this.PopBlock(block);
        $(block.DOM).remove();
    }

    public PopBlock(block: Block): Block {
        var output: Block;
        for (var i = this.Blocks.length; i >= 0; i--) {
            if (this.Blocks[i] === block) {
                var rest = this.Blocks.splice(i + 1);
                var output = this.Blocks.pop();
                this.Blocks = this.Blocks.concat(rest);
                break;
            }
        }
        return output;
    }

    //remove all blocks from DOM and their reference in this.Blocks
    public RemoveAllBlocks() {
        if (this.Blocks.length != 0) {
            var tempBlocks = this.Blocks;
            tempBlocks.forEach((block, index, array) => { this.RemoveBlock(block);})
        }
    }


    public MoveBlockToByID(blockID: string, timeline: ScenarioTimeline, InsertAfterIndex) {
        //find Block
        var block = this.GetBlockByID(blockID);
        this.MoveBlockTo(block, timeline, InsertAfterIndex);
    }

    public MoveBlockTo(block: Block, target: ScenarioTimeline, InsertAfterIndex : number) {
        this.PopBlock(block);
        target.InsertBlockAfterIndex(InsertAfterIndex, block);
    }

    public InsertDropSpaces(callback) {
        this.Blocks.forEach((block, index, array) => {
            var callbacks = new Array<Function>();
            callbacks.push((event: JQueryUI.DroppableEvent, ui: JQueryUI.DroppableEventUIParam, droppable: HTMLElement) => {
                //ui.draggable[0]
                var elem = ui.draggable[0];

                var timelineType = elem.getAttribute("data-timeline-type");
                var source: ScenarioTimeline = window.ScenarioDOM[timelineType];
                var target: ScenarioTimeline = window.ScenarioDOM[droppable.getAttribute("data-timeline-type")];
                var afterIndex = droppable.getAttribute("data-after-index");

                //Get Block object
                if (timelineType != BlockTypeEnum.None) { //TODO FIX THIS. "None" should not be a special case
                    //move block from one timeline to an other
                    source.MoveBlockToByID(elem.id, target, afterIndex);
                } else {
                    throw "not implemented";
                }
            });

            callbacks.push(callback);

            var DropArea = new DropSpaceTimedBlock(this.Id + 'drop_' + index, index, callbacks, this.Id);
            var DOM = DropArea.GetDOM();
            $(DOM).insertAfter($(block.DOM));
            DOM.style.display = "none";
            $(DOM).slideDown();
            
        });

    }

    public RemoveDropSpaces() {
        $(this.DOMWrapper).find(".dropArrow").each((index, elem) => {
            $(elem).remove();
        });
    }

    constructor(id: string, model: Array<ScenarioBlock>, container: HTMLElement, timelineType: string) {
        this.Id = id;
        this.Models = model;
        this.DOMWrapper = container;
        this.TimelineType = timelineType;
        this.Blocks = new Array<Block>();
    }


    public PopulateBlocks(blockType: string) {
        this.Blocks = new Array<Block>();
        //push initial block
        this.InsertBlockAfterIndex(-1, new StartBlock(this.Id + "_start"));


        //populate with blocks created from models.
        this.Models.forEach((blockModel, index, array) => {
            this.InsertBlockAfterIndex(index, new DeviceBlock(blockModel, blockModel.ServiceID + "_block_" + index, blockType));
        });
    }
    

    public GetInitialTimeline(id: string) : HTMLElement {
        throw "Not implemented in subclass";
    }

    public Render() {
        throw "not implemented in subclass";
    }

}

class WhenScenarioTimeline extends ScenarioTimeline {
    constructor(id: string, model: Array<ScenarioEvent>, container: HTMLElement, timelineType: string) {
        super(id, model, container, timelineType);
    }

    //populate wrapper with DOM
    public Render() {
        this.DOMWrapper.appendChild(this.GetInitialTimeline());
    }

    
    public GetInitialTimeline() {
        var output = document.createElement('div');
        output.id = this.Id;

        //Initial element
        
        this.PopulateBlocks(this.TimelineType);
        this.InsertDropSpaces(() => { });
        /*this.InsertDropSpaces((event : JQueryUI.DroppableEvent, ui: JQueryUI.DroppableEventUIParam, droppable : HTMLElement) => {
            //ui.draggable[0]
            var elem = ui.draggable[0];

            var timelineType = elem.getAttribute("data-timeline-type");
            var source: ScenarioTimeline = window.ScenarioDOM[timelineType];
            var target: ScenarioTimeline = window.ScenarioDOM[droppable.getAttribute("data-timeline-type")];
            var afterIndex = droppable.getAttribute("data-after-index");
            
            //Get Block object
            if(timelineType != BlockTypeEnum.None) { //TODO FIX THIS. "None" should not be a special case
                //move block from one timeline to an other
                source.MoveBlockToByID(elem.id, target, afterIndex);
                
            }


        });*/
        
        return output;
    }
} 


class ThenScenarioTimeline extends ScenarioTimeline {
    constructor(id: string, model: Array<ScenarioAction>, container: HTMLElement, timelineType : string) {
        super(id, model, container, timelineType);
    }

    public Render() {
        this.DOMWrapper.appendChild(this.GetInitialTimeline());
    }

    public GetInitialTimeline() {
        var output = document.createElement('div');
        output.id = this.Id;

        //Initial element

        this.PopulateBlocks(BlockTypeEnum.Then);
  
        return output;

    }
}
