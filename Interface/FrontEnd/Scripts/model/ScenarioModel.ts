

class Scenario {
    private _id: string;

    private _name: string;

    private _when: Array<ScenarioEvent>;
    public get When() { return this._when; }
    public set When(val) { this._when = val; }

    private _if: Array<ScenarioCondition>; 
    public get If() { return this._if; }
    public set If(val) { this._if = val; }

    private _then: Array<ScenarioAction>;
    public get Then() { return this._then; }
    public set Then(val) { this._then = val;}

    constructor(whenArray, ifArray, thenArray) {
        this.When = whenArray;
        this.If = ifArray;
        this.Then = thenArray;
    }
}

class ScenarioBlock {

    private _deviceID: string; 
    public get DeviceID() { return this._deviceID; }
    public set DeviceID(val) { this._deviceID = val;}

    private _serviceID: string;
    public get ServiceID() { return this._serviceID; }
    public set ServiceID(val) { this._serviceID = val;}

    private _value: string;
    public get Value() { return this._value; }
    public set Value(val) { this._value = val; }

    constructor(serviceID, value) {
        this.ServiceID = serviceID;
        this.Value = value;
        this.DeviceID = "[STATICDEVICEID]";
    }
} 

class ScenarioEvent extends ScenarioBlock {
    private _seqNumber: number;
    private get SeqNumber() { return this._seqNumber; }
    private set SeqNumber(val) { this._seqNumber = val; }

    private _duration: string;
    public get Duration() { return this._duration; }
    public set Duration(val) { this._duration = val; }

    private _timeRelation: string;
    public get TimeRelation() { return this._timeRelation; }
    public set TimeRelation(val) { this._timeRelation = val; }
    constructor(serviceID, value, SequenceNumber, duration, timeRelation) {
        super(serviceID, value);

        this.SeqNumber = SequenceNumber;
        this.Duration = duration;
        this.TimeRelation = timeRelation;
    }
}


class ScenarioCondition extends ScenarioBlock {
    constructor(serviceID, value) {
        super(serviceID, value);
    }
}

class ScenarioAction extends ScenarioBlock {
    private _seqNumber: number;
    private get SeqNumber() { return this._seqNumber; }
    private set SeqNumber(val) { this._seqNumber = val; }

    constructor(serviceID, value, seqNumber) {
        super(serviceID, value);
        this.SeqNumber = seqNumber;
    }
}

