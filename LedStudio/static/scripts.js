// vars for sample display
const blockSize = 80; // size of moving block (10x10 "pixels")
let movingBlocks = [
    {x: 0, y: 5, dx: 5, dy: -5, color: "blue"},
    {x: 300, y: 200, dx: -5, dy: 5, color: "red"},
    {x: 100, y: 150, dx: 5, dy: 5, color: "green"}
]
const SAMPLE_DISPLAY_CANVAS_ID = "sampleScreenCanvas";
const SPEED = 5;

class LedStrip {
    constructor(id) {
        this.numLeds = 20;
        this.id = id;
        this.prevStripId = 0;
        this.nextStripId = 0;
    }

    updateLedAmount(leds) {
        this.numLeds = leds;
    }
}

// global vars
var dragArea;
let ledStipCount = 0;
var ledstripBoxes = [];
var activePreviewMode = "bouncingBlocks";

function resetConfigurationView() {
    for (let box of ledstripBoxes) {
        box.remove();
    }
    ledStipCount = 0;
}

function randomIntFromInterval(min, max) { // min and max included
  return Math.floor(Math.random() * (max - min + 1) + min);
}

function changePreviewMode(event) {
    console.log("Changing preview mode to", event.target.value);
    activePreviewMode = event.target.value;
    if (activePreviewMode.includes("static")) {
        // one of the static color options was selected
        document.getElementById("fillPercentContainer").removeAttribute("style");
    } else {
        document.getElementById("fillPercentContainer").setAttribute('style', 'display: none');
    }
    updatePreviews(); //dont wait for next scheduled update
}

function checkIfDirectionneedsChange(blockpos, canvas) {
    if (blockpos.x <= 0) {
        // block is in left side needs to start moving to right
        blockpos.dx = 1 * SPEED;
    } else if (blockpos.x + blockSize >= canvas.width) {
        // block is in right side so needs to start moving to left
        blockpos.dx = -1 * SPEED;
    }

    if (blockpos.y <= 0) {
        // block is at the top, needs to start moving down
        blockpos.dy = 1 * SPEED;
    } else if (blockpos.y + blockSize >= canvas.height) {
        // block is at the bottom, needs to start moving up
        blockpos.dy = -1 * SPEED;
    }
}

function updateSampleDisplayBouncyBlocks(canvas, ctx) {
    function placeBlock(x, y, color) {
        ctx.fillStyle = color;
        ctx.fillRect(x, y, blockSize, blockSize);
    }

    ctx.fillStyle = "black";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    for (let block of movingBlocks) {
        checkIfDirectionneedsChange(block, canvas);
        block.x += block.dx;
        block.y += block.dy;
        placeBlock(block.x, block.y, block.color);
    }
}

function drawCanvasForStaticColorPreview(sliderVal) {
    const canvas = document.getElementById(SAMPLE_DISPLAY_CANVAS_ID);
    const ctx = canvas.getContext("2d");
    ctx.fillStyle = "black";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    endFillX = (parseFloat(sliderVal) / 100) * canvas.width;
    if (activePreviewMode.includes("Red")) {
        ctx.fillStyle = "#ff0000";
    } else if (activePreviewMode.includes("Green")) {
        ctx.fillStyle = "#00ff00";
    } else if (activePreviewMode.includes("Blue")) {
        ctx.fillStyle = "#0000ff";
    }
    ctx.fillRect(0, 0, endFillX, canvas.height);
}

function updateSampleDisplayStatic() {
    percentVal = document.getElementById("fillPercentSlider").value;
    document.getElementById("fillPercentSliderValue").textContent = percentVal + "%";
    if (!activePreviewMode.includes("static")) {
        return;
    }
    drawCanvasForStaticColorPreview(percentVal);
}

function updateSampleDisplay() {
    const canvas = document.getElementById(SAMPLE_DISPLAY_CANVAS_ID);
    const ctx = canvas.getContext("2d");
    
    if (activePreviewMode == "bouncingBlocks") {
        updateSampleDisplayBouncyBlocks(canvas, ctx); // this needs to update on regular basis
    }
}

function updatePreviews() {
    updateSampleDisplay()
    // TODO: update led strip previews
}

function makeDraggable(el) {
    let offsetX, offsetY, isDragging = false;

    el.addEventListener('mousedown', (e) => {
      isDragging = true;
      offsetX = e.clientX - el.offsetLeft;
      offsetY = e.clientY - el.offsetTop;
      el.style.zIndex = 1000;
    });

    el.addEventListener('mouseup', (e) => {
      isDragging = false;
    });

    document.addEventListener('mousemove', (e) => {
      if (!isDragging) return;

      const x = e.clientX - offsetX;
      const y = e.clientY - offsetY;

      const maxX = dragArea.clientWidth - el.offsetWidth;
      const maxY = dragArea.clientHeight - el.offsetHeight;

      el.style.left = Math.min(Math.max(0, x), maxX) + 'px';
      el.style.top = Math.min(Math.max(0, y), maxY) + 'px';
    });
}


$(document).ready(function() {
    // setup "display" preview canvas
    const canvas = document.getElementById(SAMPLE_DISPLAY_CANVAS_ID);
    const ctx = canvas.getContext("2d");
    ctx.fillSyle = "black";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    setInterval(updatePreviews, 40);

    document.getElementById('previewModeSelection').addEventListener("change", changePreviewMode);
    document.getElementById('fillPercentSlider').addEventListener("input", updateSampleDisplayStatic);

    dragArea = document.getElementById('configurationArea');
    document.getElementById('addStripButton').addEventListener('click', () => {
        
        const box = document.createElement('div');
        box.className = 'draggable-box';

        const canvas = document.createElement('canvas');
        canvas.width = 300; // internal canvas resolution
        canvas.height = 20;

        const bottomRow = document.createElement('div');
        bottomRow.className = 'bottom-row';

        // Label
        const label = document.createElement('div');
        label.className = 'strip-label';
        label.textContent = 'Strip #' + (++ledStipCount);

        // Input
        const input = document.createElement('input');
        input.type = 'number';
        input.min = 0;
        input.value = 100;
        input.className = 'lednum-input';

        // Delete button
        const deleteBtn = document.createElement('button');
        deleteBtn.textContent = '✕';
        deleteBtn.className = 'delete-btn';
        deleteBtn.addEventListener('click', (e) => {
            e.stopPropagation(); // prevent triggering drag
            box.remove();
        });

        box.appendChild(canvas);
        bottomRow.appendChild(label);
        bottomRow.appendChild(input);
        bottomRow.appendChild(deleteBtn);
        box.appendChild(bottomRow);

        // Random position inside the area
        box.style.left = Math.random() * (dragArea.clientWidth - 300) + 'px';
        box.style.top = Math.random() * (dragArea.clientHeight - 150) + 'px';

        makeDraggable(box);
        dragArea.appendChild(box);
        ledstripBoxes.push(box);
    })
});