import React, { useState, useEffect } from "react";
import "./App.css";

function polarToCartesian(degrees, distance) {
  // Convert degrees to radians
  var radians = degrees * (Math.PI / 180);

  // Calculate Cartesian coordinates
  var x = distance * Math.cos(radians);
  var y = distance * Math.sin(radians);

  // Return Cartesian coordinates as an object
  return { x: x, y: y };
}

function convertPolarToCartesianArray(polarCoordinates) {
  var cartesianCoordinates = [];

  // Iterate through polar coordinates array
  for (var i = 0; i < polarCoordinates.length; i++) {
    var polar = polarCoordinates[i];
    var degrees = polar[0];
    var distance = polar[1];

    // Convert polar coordinates to Cartesian
    var cartesian = polarToCartesian(degrees, distance);

    // Add Cartesian coordinates to the result array
    cartesianCoordinates.push(cartesian);
  }

  return cartesianCoordinates;
}

function rotatePoints(polarCoordinates, angle, direction) {
  const rotatedCoordinates = polarCoordinates.map(([degrees, distance]) => {
    // Adjust angle based on direction
    const rotatedAngle =
      direction === "right" ? degrees - angle : degrees + angle;
    // Ensure angle is within 0 to 360 degrees range
    const finalAngle = (rotatedAngle + 360) % 360;
    return [finalAngle, distance];
  });
  return convertPolarToCartesianArray(rotatedCoordinates);
}

function App() {
  const [bluePolarCoordinates] = useState([
    [153, 48.47],
    [111, 37.37],
    [69, 56.6],
    [34, 52.83],
    [19, 43.77],
    [18, 55.95],
    [0, 65.96],
  ]);

  const [redPolarCoordinates] = useState([
    [180, 48.47],
    [140, 37.37],
    [90, 56.6],
    [60, 52.83],
    [30, 43.77],
    [10, 55.95],
    [0, 65.96],
  ]);

  const [blueDots, setBlueDots] = useState([]);
  const [redDots, setRedDots] = useState([]);
  const [currentRotation, setCurrentRotation] = useState(0); // Track current rotation

  useEffect(() => {
    // Render the initial points when the component mounts
    renderPoints(convertPolarToCartesianArray(bluePolarCoordinates), "blue");
    renderPoints(convertPolarToCartesianArray(redPolarCoordinates), "red");
  }, []); // Empty dependency array ensures it only runs once on mount

  function renderPoints(points, color) {
    // Update the dots state with new points
    if (color === "blue") {
      setBlueDots(
        points.map((coordinate, index) => (
          <circle
            key={index}
            cx={coordinate.x * 4 + 400} // Adjusting for the SVG coordinates
            cy={400 - coordinate.y * 4} // Adjusting for the SVG coordinates
            r="2" // Radius of the dot
            fill="blue" // Color of the dot
            filter="drop-shadow(0 0 2px blue)"
          />
        ))
      );
    } else if (color === "red") {
      setRedDots(
        points.map((coordinate, index) => (
          <circle
            key={index}
            cx={coordinate.x * 4 + 400} // Adjusting for the SVG coordinates
            cy={400 - coordinate.y * 4} // Adjusting for the SVG coordinates
            r="2" // Radius of the dot
            fill="red" // Color of the dot
            filter="drop-shadow(0 0 2px red)"
          />
        ))
      );
    }
  }

  function handleRotate() {
    // Get the input value for rotation in degrees
    const rotateInput = document.getElementById("rotateInput").value;
    // Get the selected direction
    const rotateDirection = document.getElementById("rotateDirection").value;

    // Ensure rotateInput is a valid number
    const degrees = parseFloat(rotateInput);
    if (isNaN(degrees)) {
      alert("Please enter a valid number for rotation.");
      return;
    }

    // Calculate the total rotation angle
    let totalRotation;
    if (rotateDirection === "left") {
      totalRotation = currentRotation - degrees;
    } else {
      totalRotation = currentRotation + degrees;
    }

    // Rotate the points
    const rotatedBluePoints = rotatePoints(
      bluePolarCoordinates,
      totalRotation,
      "left"
    );
    const rotatedRedPoints = rotatePoints(
      redPolarCoordinates,
      totalRotation,
      "left"
    );

    // Animate the rotation for blue and red dots separately
    animateRotation(rotatedBluePoints, "blue");
    animateRotation(rotatedRedPoints, "red");

    // Update the current rotation state
    setCurrentRotation(totalRotation);
  }

  function animateRotation(targetPoints, color) {
    // Get the dots container
    const dotsContainer = document.getElementById(`${color}DotsContainer`);
    const dots = dotsContainer.childNodes;

    // Start time of the animation
    const startTime = performance.now();

    // Initial positions of the dots
    const initialPositions = [];
    dots.forEach((dot) => {
      initialPositions.push({
        x: parseFloat(dot.getAttribute("cx")),
        y: parseFloat(dot.getAttribute("cy")),
      });
    });

    // Duration of the animation (in milliseconds)
    const duration = 1000; // 1 second

    // Animation function
    function update() {
      const elapsed = performance.now() - startTime;
      const progress = Math.min(elapsed / duration, 1);

      // Update dot positions
      dots.forEach((dot, index) => {
        const startX = initialPositions[index].x;
        const startY = initialPositions[index].y;
        const targetX = targetPoints[index].x * 4 + 400;
        const targetY = 400 - targetPoints[index].y * 4;

        const newX = startX + (targetX - startX) * progress;
        const newY = startY + (targetY - startY) * progress;

        dot.setAttribute("cx", newX);
        dot.setAttribute("cy", newY);
      });

      // Continue animation if not finished
      if (progress < 1) {
        requestAnimationFrame(update);
      }
    }

    // Start the animation
    requestAnimationFrame(update);
  }

  return (
    <>
      <div className="header">
        <h1>CyGUI</h1>
      </div>
      <div className="container">
        <div className="radar-container">
          <svg
            width="800"
            height="400"
            style={{
              borderRadius: "10px",
              borderBottomLeftRadius: "0",
              borderBottomRightRadius: "0",
              border: "1px solid darkslategray",
              background: "black",
            }}
          >
            <circle cx="50%" cy="100%" r="400" fill="#033E3E">
              <title>Background</title>
            </circle>
            <g>
              <circle
                cx="50%"
                cy="100%"
                r="400"
                fill="transparent"
                stroke="darkgreen"
                strokeWidth="1"
              >
                <title>75cm-100cm</title>
              </circle>
              <text
                x="0%"
                y="100%"
                fill="darkgreen"
                font-size="10"
                fontFamily="sans-serif"
              >
                100cm
              </text>
              <circle
                cx="50%"
                cy="100%"
                r="300"
                fill="transparent"
                stroke="darkgreen"
                strokeWidth="1"
              >
                <title>50cm-75cm</title>
              </circle>
              <text
                x="12.5%"
                y="100%"
                fill="darkgreen"
                font-size="10"
                fontFamily="sans-serif"
              >
                75cm
              </text>
              <circle
                cx="50%"
                cy="100%"
                r="200"
                fill="transparent"
                stroke="darkgreen"
                strokeWidth="1"
              >
                <title>25cm-50cm</title>
              </circle>
              <text
                x="25%"
                y="100%"
                fill="darkgreen"
                font-size="10"
                fontFamily="sans-serif"
              >
                50cm
              </text>
              <circle
                cx="50%"
                cy="100%"
                r="100"
                fill="transparent"
                stroke="darkgreen"
                strokeWidth="1"
              >
                <title>0cm-25cm</title>
              </circle>
              <text
                x="37.5%"
                y="100%"
                fill="darkgreen"
                font-size="10"
                fontFamily="sans-serif"
              >
                25cm
              </text>
            </g>
            <g>
              <line
                x1="400"
                y1="400"
                x2="400"
                y2="0"
                style={{ stroke: "darkgreen", strokeWidth: "1" }}
              >
                <title>90deg</title>
              </line>
              <line
                x1="400"
                y1="400"
                x2="746.41"
                y2="200"
                style={{ stroke: "darkgreen", strokeWidth: "1" }}
              >
                <title>30deg</title>
              </line>
              <line
                x1="400"
                y1="400"
                x2="53.59"
                y2="200"
                style={{ stroke: "darkgreen", strokeWidth: "1" }}
              >
                <title>150deg</title>
              </line>
              <line
                x1="400"
                y1="400"
                x2="600"
                y2="53.59"
                style={{ stroke: "darkgreen", strokeWidth: "1" }}
              >
                <title>60deg</title>
              </line>
              <line
                x1="400"
                y1="400"
                x2="200"
                y2="53.59"
                style={{ stroke: "darkgreen", strokeWidth: "1" }}
              >
                <title>120deg</title>
              </line>
              <line
                x1="400"
                y1="400"
                x2="682.843"
                y2="117.157"
                style={{ stroke: "darkgreen", strokeWidth: "1" }}
              >
                <title>45deg</title>
              </line>
              <line
                x1="400"
                y1="400"
                x2="117.157"
                y2="117.157"
                style={{ stroke: "darkgreen", strokeWidth: "1" }}
              >
                <title>135deg</title>
              </line>
            </g>
            <circle cx="50%" cy="460" r="70" fill="gray">
              <title>CyBot</title>
            </circle>
            <g id="blueDotsContainer">{blueDots}</g>
            <g id="redDotsContainer">{redDots}</g>
          </svg>
          <div className="control-panel">
            <p>Control Panel</p>
            <div className="grid-container">
              <div className="controls-container">
                <details>
                  <summary>Rotation</summary>
                  <hr />
                  <div className="controls">
                    Rotate{" "}
                    <input
                      type="text"
                      id="rotateInput"
                      style={{ width: "25px" }}
                    />{" "}
                    degrees to the{" "}
                    <select id="rotateDirection">
                      <option value="left">left</option>
                      <option value="right">right</option>
                    </select>
                    .<br />
                    <br />
                    <button onClick={handleRotate}>Send Command</button>
                  </div>
                </details>
              </div>
              <div className="controls-container">
                <details>
                  <summary>Drive</summary>
                  <hr />
                  <div className="controls">
                    Drive <input type="text" style={{ width: "25px" }} />
                    cm&nbsp;
                    <select>
                      <option value="forward">forward</option>
                      <option value="backward">backwards</option>
                    </select>
                    .
                    <br />
                    <br />
                    <button>Send Command</button>
                  </div>
                </details>
              </div>
              <div className="controls-container">
                <details>
                  <summary>Scan</summary>
                  <hr />
                  <div className="controls">
                    Scan Parameters:
                    <br />
                    From <input type="text" style={{ width: "25px" }} />
                    deg to <input type="text" style={{ width: "25px" }} />
                    deg.
                    <br />
                    Pulse every <input type="text" style={{ width: "25px" }} />
                    deg.
                    <br />
                    <br />
                    <button>Send Command</button>
                  </div>
                </details>
              </div>
            </div>
          </div>
        </div>
      </div>
    </>
  );
}

export default App;
