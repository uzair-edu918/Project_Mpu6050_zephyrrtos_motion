import './App.css';
import React, { useEffect, useState, useRef } from 'react';
import axios from 'axios';
import { Canvas } from '@react-three/fiber'

import {  useFrame } from '@react-three/fiber';
import * as THREE from 'three';
const normalize = (value, min, max, scaleMin, scaleMax) => {
  return ((value - min) / (max - min)) * (scaleMax - scaleMin) + scaleMin;
};

const Cube = ({ rotation }) => {
  const meshRef = useRef();

  useFrame(() => {
    if (meshRef.current) {
      meshRef.current.rotation.x = rotation[0];
      meshRef.current.rotation.y = rotation[1];
    }
  });

  return (
    <mesh ref={meshRef} castShadow position={[0, 1, 0]}>
      <boxGeometry args={[2, 2, 2]} />
      <meshStandardMaterial color="black" />
    </mesh>
  );
};
const Scene = () => {
  const [rotation, setRotation] = useState([0, 0]);


  useEffect(() => {
    const fetchData = async () => {
      try {


        const ws = new WebSocket('ws://localhost:8080');

          ws.onopen = () => {
            console.log('Connected to WebSocket server');
        };
        
        ws.onmessage = (event) => {
          // console.log('Received message:', event.data);
          const lines = event.data.split('\n');
          console.log('Received message:', lines);
          const X = lines[1];
          const Y = lines[2];
          const Z = lines[3];
          const normalizedX = normalize(X, -16384, 16384, -5, 5);
          const normalizedY = normalize(Y, -16384, 16384, -5, 5);
          const normalizedZ = normalize(Z, -16384, 16384, -5, 5); 
          console.log(normalizedX, normalizedY, normalizedZ)
          const roll = Math.atan2(Y, Z);
          const pitch = Math.atan2(-X, Math.sqrt(Y * Y + Z * Z));
          console.log(normalizedX, normalizedY, normalizedZ)
          setRotation([roll, pitch]);
          // setMessage(event.data);  // Update state with the current message
      };

        ws.onclose = () => {
          console.log('Disconnected from WebSocket server');
      };

      ws.onerror = (error) => {
          console.error('WebSocket error:', error);
      };
        
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };

    fetchData();


  }, []);

  return (
    <Canvas shadows   onCreated={({ gl }) => {
      gl.shadowMap.enabled = true;
      gl.shadowMap.type = THREE.PCFSoftShadowMap; // Use soft shadows
    }}>
     <ambientLight intensity={0.5 * Math.PI} /> 
     <spotLight position={[10, 10, 10]} angle={0.15} penumbra={1} decay={0} intensity={Math.PI} />
    <directionalLight
      castShadow
      position={[5, 10, 5]}
      intensity={1}
      shadow-mapSize-width={2048}
      shadow-mapSize-height={2048}
      shadow-camera-near={0.5}
      shadow-camera-far={20}
      shadow-camera-left={-10}
      shadow-camera-right={10}
      shadow-camera-top={10}
      shadow-camera-bottom={-10}
    />
          <pointLight position={[-5, -5, -5]} intensity={0.5} />
      <Cube rotation={rotation} />
      <Ground />

      <mesh receiveShadow rotation={[-Math.PI / 2, 0, 0]} position={[0, -1, 0]}>
        <planeGeometry args={[20, 20]} />
        <shadowMaterial opacity={1} />
      </mesh>
    </Canvas>
  );
};

const Ground = () => (
  <mesh receiveShadow rotation={[-Math.PI / 2, 0, 0]} position={[0, -1, 0]}>
    <planeGeometry args={[10, 10]} />
    <shadowMaterial opacity={0.5} />
  </mesh>
);
function App() {
  // const [data, setData] = useState(null);
  // const isFetching = useRef(false); // Use a ref to keep track of request status

  // useEffect(() => {

  //   const fetchData = async () => {
  //     if (isFetching.current) return; // Prevent sending a new request if one is in progress
  //     isFetching.current = true; // Set flag to indicate request in progress
  //     try {
  //       const response = await axios.get('http://192.168.100.70:8080/').then(response => {
  //         const { data } = response;
  //         console.log(data); // This should log the data from the response
  //         setData(data);
  //       });
  //     } catch (error) {
  //       console.error('Error fetching data:', error);
  //     } finally {
  //       isFetching.current = false; // Reset flag when request is complete
  //     }

  //   }
  //   const intervalId = setInterval(fetchData, 10);

  //   // Cleanup interval on component unmount
  //   return () => clearInterval(intervalId);

  // }, []);



  return (
    <div className="App">

      {/* <div> */}
        {/* <h1>Data from API:</h1> */}
        {/* {positit ? <pre>{JSON.stringify(data, null, 2)}</pre> : <p>Loading...</p>} */}
      {/* </div> */}
      {/* <Scene></Scene> */}

      <div style={{ height: '100vh', width: '100vw' }}>
     

      <Scene />

    </div>
{/* 
      <div id="canvas-container">
        <Canvas>
          <ambientLight intensity={0.1} />
          <directionalLight color="red" position={[0, 0, 5]} />
          <mesh>
            <boxGeometry args={[5, 5, 5]}/>
            <meshStandardMaterial />
          </mesh>
        </Canvas>




      </div> */}


    </div>
  );
}

export default App;
