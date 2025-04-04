// ESP32MotorControl.jsx
import React, { useEffect, useRef, useState } from 'react';
import mqtt from 'mqtt';

const ESP32MotorControl = () => {
    // MQTT broker y topics
    const brokerUrl = 'wss://broker.emqx.io:8084/mqtt'; // URL del broker MQTT público
    const topicControl = 'motor/control'; // Tópico para controlar el motor (dirección)
    const topicSelect = 'motor/select';   // Tópico para seleccionar el motor

    // Estados
    const [client, setClient] = useState(null);         // Cliente MQTT
    const [selectedMotor, setSelectedMotor] = useState(1); // Motor seleccionado por defecto
    const [startTime, setStartTime] = useState(0);      // Tiempo de inicio de movimiento del motor
    const logRef = useRef(null);                        // Referencia al textarea para logs

    // Conexión inicial al broker MQTT al montar el componente
    useEffect(() => {
        const mqttClient = mqtt.connect(brokerUrl); // Conectar al broker

        mqttClient.on('connect', () => {
            console.log('Connected to MQTT Broker');
        });

        mqttClient.on('error', (err) => {
            console.error('MQTT Error:', err);
        });

        setClient(mqttClient); // Guardar cliente en estado

        // Cierre limpio al desmontar el componente
        return () => {
            mqttClient.end();
        };
    }, []);

    // Agrega un mensaje al área de logs
    const log = (message) => {
        if (logRef.current) {
            logRef.current.value += `${message}\n`; // Añadir nueva línea
            logRef.current.scrollTop = logRef.current.scrollHeight; // Scroll automático al final
        }
    };

    // Envía un comando al tópico de control
    const sendCommand = (command) => {
        if (client?.connected) {
            client.publish(topicControl, command); // Publicar comando
            console.log(`Sent: ${topicControl} -> ${command}`);
        } else {
            console.error('MQTT not connected!');
        }
    };

    // Selecciona un motor y lo publica al broker
    const handleSelectMotor = (motorNumber) => {
        setSelectedMotor(motorNumber); // Guardar motor seleccionado
        if (client?.connected) {
            client.publish(topicSelect, String(motorNumber)); // Publicar selección
            console.log(`Selected Motor: ${motorNumber}`);
        }
        log(`Selected Motor: ${motorNumber}`); // Mostrar en logs
    };

    // Inicia el motor en una dirección (FORWARD o BACKWARD)
    const handleStartMotor = (direction) => {
        setStartTime(Date.now()); // Guardar momento en que inicia
        sendCommand(`${selectedMotor}:${direction}`); // Publicar comando con dirección
        log(`Motor ${selectedMotor} started: ${direction}`);
    };

    // Detiene el motor y calcula duración de movimiento
    const handleStopMotor = () => {
        const endTime = Date.now();
        const duration = ((endTime - startTime) / 1000).toFixed(2); // Duración en segundos
        sendCommand(`${selectedMotor}:STOP`); // Enviar comando STOP
        log(`Motor ${selectedMotor} stopped. Duration: ${duration} seconds`);
    };

    return (
        <div className="min-h-screen bg-[#1e1e2f] text-white p-6 text-center font-sans">
            <h1 className="text-3xl font-bold text-[#00d1b2] mb-6">Control de Múltiples Motores con ESP32</h1>

            <div className="flex flex-col items-center gap-6">
                {/* Botones de selección de motor */}
                <div className="flex flex-wrap justify-center gap-4">
                    <button className="bg-[#e74c3c] text-white py-3 px-6 text-lg rounded-lg shadow-md hover:opacity-80 hover:scale-105 transition" onClick={() => handleSelectMotor(1)}>Motor 1</button>
                    <button className="bg-[#3498db] text-white py-3 px-6 text-lg rounded-lg shadow-md hover:opacity-80 hover:scale-105 transition" onClick={() => handleSelectMotor(2)}>Motor 2</button>
                    <button className="bg-[#f39c12] text-white py-3 px-6 text-lg rounded-lg shadow-md hover:opacity-80 hover:scale-105 transition" onClick={() => handleSelectMotor(3)}>Motor 3</button>
                    <button className="bg-[#2ecc71] text-white py-3 px-6 text-lg rounded-lg shadow-md hover:opacity-80 hover:scale-105 transition" onClick={() => handleSelectMotor(4)}>Motor 4</button>
                </div>

                {/* Botones de dirección del motor */}
                <div className="flex flex-wrap justify-center gap-4">
                    <button
                        className="bg-[#9b59b6] text-white py-3 px-6 text-lg rounded-lg shadow-md hover:opacity-80 hover:scale-105 transition"
                        onMouseDown={() => handleStartMotor('FORWARD')} // Iniciar hacia adelante
                        onMouseUp={handleStopMotor}                    // Detener al soltar
                    >
                        ADELANTE
                    </button>
                    <button
                        className="bg-[#dc3545] text-white py-3 px-6 text-lg rounded-lg shadow-md hover:opacity-80 hover:scale-105 transition"
                        onMouseDown={() => handleStartMotor('BACKWARD')} // Iniciar hacia atrás
                        onMouseUp={handleStopMotor}                      // Detener al soltar
                    >
                        ATRÁS
                    </button>
                </div>

                {/* Área de logs de eventos */}
                <textarea
                    ref={logRef}
                    readOnly
                    className="w-[90%] max-w-xl h-40 mt-4 p-3 bg-[#2e2e3e] text-white border border-[#00d1b2] rounded-lg font-mono text-sm resize-none"
                ></textarea>
            </div>
        </div>
    );
};

export default ESP32MotorControl;
