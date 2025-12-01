# Trackspacer - Spectral Ducking Plugin

Un clon completo del plugin Trackspacer que implementa ducking espectral dinámico.

## 🎛️ Características

- **Entrada Sidechain** - Configuración estéreo para señal principal y sidechain
- **Procesamiento FFT** - Análisis espectral con ventanas de 2048 muestras
- **Ducking Dinámico** - Reducción de ganancia por banda de frecuencia
- **Visualización en Tiempo Real** - Analizador de espectro con input, sidechain y gain reduction
- **Controles Completos**: Amount, Attack, Release, Low/High Freq, Smooth, Solo, Bypass

## 📥 Compilar en Windows (Paso a Paso)

### Requisitos Previos

1. **JUCE Framework**
   - Descarga desde: https://juce.com/get-juce/download
   - Versión recomendada: JUCE 7.x o superior
   - Instala el Projucer

2. **Visual Studio 2022** (o 2019)
   - Community Edition es suficiente (gratis)
   - Asegúrate de tener instalado "Desktop development with C++"

### Pasos de Compilación

#### Opción A: Con Projucer (Recomendado - Más Fácil)

1. **Descarga el repositorio:**
   ```powershell
   git clone https://github.com/fertomusic-creator/JUCE-FER.git
   cd JUCE-FER
   git checkout claude/trackspacer-audio-plugin-01EVF4Jb5k86oiKvkGkuLSx9
   ```

2. **Abre el proyecto en Projucer:**
   - Abre Projucer
   - File → Open → Selecciona `JUCE-FER/Trackspacer/Trackspacer.jucer`

3. **Configura la ruta de JUCE:**
   - En Projucer, ve a Settings (icono de engranaje)
   - En "Global Paths", establece "Path to JUCE" a la carpeta donde instalaste JUCE
   - Ejemplo: `C:\JUCE`

4. **Exporta el proyecto:**
   - En Projucer, haz clic en el botón "Save and Open in IDE"
   - Esto abrirá Visual Studio automáticamente

5. **Compila en Visual Studio:**
   - En Visual Studio, selecciona la configuración: **Release** y **x64**
   - Build → Build Solution (o presiona F7)
   - Espera a que compile (puede tomar 5-10 minutos la primera vez)

6. **Encuentra tu VST3:**
   ```
   JUCE-FER/Trackspacer/Builds/VisualStudio2022/x64/Release/VST3/Trackspacer.vst3
   ```

#### Opción B: Con CMake

1. **Descarga el repositorio:**
   ```powershell
   git clone https://github.com/fertomusic-creator/JUCE-FER.git
   cd JUCE-FER/Trackspacer
   ```

2. **Genera el proyecto de Visual Studio:**
   ```powershell
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

3. **Compila:**
   ```powershell
   cmake --build . --config Release
   ```

4. **El VST3 estará en:**
   ```
   JUCE-FER/Trackspacer/build/Trackspacer_artefacts/VST3/Trackspacer.vst3
   ```

## 📂 Instalar el VST3

Una vez compilado, copia la carpeta `Trackspacer.vst3` a una de estas ubicaciones:

- **Ubicación del usuario:** `C:\Users\TuNombre\AppData\Local\Programs\Common\VST3\`
- **Ubicación del sistema:** `C:\Program Files\Common Files\VST3\`

## 🎵 Usar el Plugin

1. **Abre tu DAW** (Ableton, FL Studio, Reaper, etc.)
2. **Escanea nuevos plugins** (Rescan VST3 plugins)
3. **Carga Trackspacer** en una pista
4. **Configura el sidechain:**
   - Input principal: La señal que quieres procesar (ej: bajo)
   - Sidechain: La señal de referencia (ej: kick)
5. **Ajusta los parámetros:**
   - **Amount**: Intensidad del ducking
   - **Attack/Release**: Velocidad de respuesta
   - **Low/High Freq**: Rango de frecuencias a procesar

## 🔧 Solución de Problemas

### "No puedo abrir el archivo .jucer"
- Asegúrate de tener Projucer instalado
- Descarga JUCE desde https://juce.com/get-juce/download

### "Error de compilación: No se encuentra JUCE"
- En Projucer: Settings → Global Paths → Path to JUCE
- Apunta a tu carpeta de JUCE (ej: `C:\JUCE`)

### "El plugin no aparece en mi DAW"
- Verifica que copiaste Trackspacer.vst3 a la carpeta correcta
- Escanea nuevos plugins en tu DAW
- Algunos DAWs tienen listas negras, revisa la configuración

### "Error al compilar: C2039 o símbolos no resueltos"
- Asegúrate de estar usando Visual Studio 2019 o 2022
- Selecciona configuración x64 (no x86)
- Limpia y recompila: Build → Clean Solution, luego Build Solution

## 📝 Arquitectura del Código

```
Trackspacer/
├── Source/
│   ├── PluginProcessor.cpp/h    # Motor principal del plugin
│   ├── PluginEditor.cpp/h       # Interfaz gráfica
│   ├── SpectralEngine.cpp/h     # Procesamiento FFT
│   └── SpectrumAnalyzer.cpp/h   # Visualización
├── Trackspacer.jucer            # Proyecto Projucer
└── CMakeLists.txt              # Proyecto CMake (alternativo)
```

## 📄 Licencia

ISC License - Ver código fuente para detalles completos.

## 🙋 Soporte

Si tienes problemas compilando, abre un issue en el repositorio de GitHub.
