

class UIHelper {

    /*public static CreateInitialBlock(id: string) {
        var initialBlock = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        initialBlock.setAttribute("xmlns", "http://www.w3.org/2000/svg");
        initialBlock.setAttribute("style", "width:50px; height:100px;");
        initialBlock.setAttribute("id", id);
        initialBlock.setAttribute("class", "blockSpacing");
        var start = document.createElementNS("http://www.w3.org/2000/svg", "polygon");
        start.setAttribute("points", "0,0 50,50 0,100 0,0");
        start.setAttribute("fill", "blue");
        start.setAttribute("class", "startTriangle block");
        initialBlock.appendChild(start);
        return initialBlock;
    }*/


    //concave end
    public static CreateStartArrow(id: string, wrapperClasses: Array<string>, childClasses: Array<string>) {
        var beginContainer = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        beginContainer.setAttribute("xmlns", "http://www.w3.org/2000/svg");
        beginContainer.setAttribute("style", "width:50px; height:100px; display:inline;");
        var beginArrow = document.createElementNS("http://www.w3.org/2000/svg", "polygon");
        beginArrow.setAttribute("points", "0,0 50,0 50,100 0,100 50,50 0,0");
        childClasses.forEach((value, index, array) => { beginArrow.setAttribute("class", value);  });
        //beginArrow.setAttribute("class", "timedBlock");
        beginContainer.appendChild(beginArrow);
        return beginContainer;
    }


    //pointy end
    public static CreateEndArrow(id: string, wrapperClasses: Array<string>, childClasses: Array<string>) {
        var endContainer = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        endContainer.setAttribute("xmlns", "http://www.w3.org/2000/svg");
        endContainer.setAttribute("style", "width:50px; height:100px; display:inline;");
        endContainer.setAttribute("id", id);
        endContainer.setAttribute("class", "blockSpacing");
        var end = document.createElementNS("http://www.w3.org/2000/svg", "polygon");
        end.setAttribute("points", "0,0 50,50 0,100 0,0");
        childClasses.forEach((value, index, array) => { end.setAttribute("class", value); });
        endContainer.appendChild(end);
        return endContainer;
    }
}