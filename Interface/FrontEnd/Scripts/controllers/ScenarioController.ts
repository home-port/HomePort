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

    public Populate() {

    }

    constructor(scenario: Scenario , whenWrapper: HTMLElement, ifWrapper: HTMLElement, thenWrapper: HTMLElement) {
        //Initialize scenario
        this.Model = scenario;
        this.When = new WhenScenarioTimeline("when", scenario.When, whenWrapper);
        this.If = new ScenarioTimeline("If", scenario.If, ifWrapper); //TODO IMPLEMENT in subclass
        this.Then = new ScenarioTimeline("Then", scenario.When, thenWrapper); //TODO IMPLEMENT in subclass


        //populate 
        this.When.Render();



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

    private blocks: Array<Block>;
    public get Blocks() { return this.blocks; }
    public set Blocks(val) { this.blocks = val; }

    //insert specific block in this.block and DOM
    public InsertBlockAfterIndex(index: number, newBlock: Block) {
        if (index >= 0) {
            var rest = this.Blocks.splice(index + 1);
            this.Blocks.push(newBlock);
            this.Blocks.concat(rest);
            //Move the DOM.
            $(newBlock.DOM).insertAfter(this.Blocks[index].DOM);
        }

        if (index < 0) {
            this.Blocks.push(newBlock);
            this.DOMWrapper.appendChild(newBlock.DOM);
        }
    }
    //remove specific index from this.blocks and DOM
    public RemoveBlockIndex(index: number) {
        var rest = this.Blocks.splice(index + 1);
        var remove = this.Blocks.pop();
        $(remove).remove();
        this.Blocks.concat(rest);
    }

    //insert block after specific block in this.blocks and DOM
    public InsertBlockAfter(after: Block, newBlock: Block) {
        for (var i = 0; i < this.Blocks.length; i++) {
            if (this.Blocks[i] === after) {
                var rest = this.Blocks.splice(i + 1);
                this.Blocks.push(newBlock);
                this.blocks.concat(rest);
                //Move the DOM.
                $(newBlock.DOM).insertAfter(after.DOM);

                break;
            }
        }
    }
    //remove specific block from DOM and this.Blocks
    public RemoveBlock(block: Block) {
        for (var i = this.Blocks.length; i >= 0; i--) {
            if (this.Blocks[i] === block) {
                var rest = this.Blocks.splice(i + 1);
                var remove = this.Blocks.pop();
                $(remove).remove();
                this.Blocks.concat(rest);
            }
        }
    }

    //remove all blocks from DOM and their reference in this.Blocks
    public RemoveAllBlocks() {
        if (this.Blocks.length != 0) {
            var tempBlocks = this.Blocks;
            tempBlocks.forEach((block, index, array) => { this.RemoveBlock(block);})
        }
    }


    public InsertDropSpaces(callback) {
        this.Blocks.forEach((block, index, array) => {
            var DropArea = new DropSpaceTimedBlock('drop_' + index);
            $(DropArea.DOM).insertAfter($(block.DOM));
        });

    }

    public RemoveDropSpaces() {

    }

    constructor(id: string, model: Array<ScenarioBlock>, container: HTMLElement) {
        this.Id = id;
        this.Models = model;
        this.DOMWrapper = container;
        this.Blocks = new Array<Block>();
    }


    public PopulateBlocks(blockType: string) {
        this.Blocks = new Array<Block>();
        //push initial block
        this.InsertBlockAfterIndex(-1, new StartBlock(this.Id + "_start"));


        //populate with blocks created from models.
        this.Models.forEach((blockModel, index, array) => {
            this.InsertBlockAfterIndex(0, new DeviceBlock(blockModel, this.Id + "_block_" + index, blockType));
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
    constructor(id: string, model: Array<ScenarioEvent>,  container: HTMLElement) {
        super(id, model, container);
    }

    //populate wrapper with DOM
    public Render() {
        this.DOMWrapper.appendChild(this.GetInitialTimeline());
    }

    

    public GetInitialTimeline() {
        var output = document.createElement('div');
        output.id = this.Id;

        //Initial element
        
        this.PopulateBlocks(BlockTypeEnum.When);
        return output;
    }
} 

