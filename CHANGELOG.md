### v 0.9.9.1

---------------------------

* Fixed: Bug in configuration file parsing
* Added: WifiObserver plugin for win32
* Reworked: Sensor layer
* Added: SetUseWifiObserver/GetUseWifiObserver, SetUseBtObserver/GetUseBtObserver to Environs object
* Updated: Wifi observer (Windows, OSX)
* Added: Android platform layer with AddObserverForSensorData / RemoveObserverForSensorData
* Added: IsSensorAvailable to Environs object to query sensor availability before subscribing
* Added: SetSensorEvent to Environs object to subscribe to sensor data from ourself
* Updated: EnvironsSensorObserver
* Fixed: Race condition in connect process


### v 0.9.9.0

---------------------------

* Updated: Environs Disposer reference counting
* Fixed: Bug in Stunt establisher
* Disabled: Mediator V6 protocol
* Added: Lock of device storage path buffer


### v 0.9.8.9

---------------------------

* Fixed: Bug in Android layer for PortalInstance (accidentally taken over cache-index of source)
* Moved: All PortalObserver to DeviceInstances
* Fixed: Bug in Portalmessage dispatching
* Updated: PortalInstance lifecycle connector from CPP to ARC
* Updated: Stunt for Android layer
* Fixed: Bug in DataStore functions of DeviceBase
* Added: OSX shared schemes.
* Updated: Creating of plugin instances
* Reworked: Start of Environs instances
* Updated: Mediator server user management
* Updated: ChatApp Windows


### v 0.9.8.8

---------------------------

* Updated: ChatApp Android (thread usage)
* Added: RWLock to Mediator layer (used in server)
* Changed: Device ports to 5959, Added: Environs.SetBasePort() to change environment port for each instance individually
* Updated: ChatApp OSX init threads
* Updated: Mediator layer compressed device ident keys


### v 0.9.8.7

---------------------------

* Added: Mediator server use cached decrypt key
* Added: Mediator layer high priority queue
* Fixed: Bug in stunt channel establisher
* Updated: Mediator client layer send thread usage
* Reworked: Concurrent stunt implementation
* Fixed: Bug in Mediator layer compress algorithm
* Updated: Disposal of concurrent stunt sockets
* Updated: Stunt locks
* Updated: Code for concurrent stunt in mediator layer


### v 0.9.8.6

---------------------------

* Added: Code for concurrent stunt in mediator layer
* Updated: Mediator server layer send thread contexts compress algorithm
* Updated: Mediator client layer send thread contexts compress algorithm
* Updated: Mediator server push to send queue with trylock
* Added: Mediator client layer send context compression
* Added: Mediator server onion listener
* Added: Lock guard for thread creation
* Updated: Disposal of poll sockets in Core and Mediator listener
* Changed: DeviceBase stunt SP to WP
* Changed: DeviceBase stun SP to WP
* Added: Mediator server send thread interlock usage
* Changed: ThreadSync.autoreset default to false
* Added: Delayed termination to GC
* Changed: Environs instance SP reference to WP
* Changed: DeviceBase SP reference to WP
* Added: Send quit message in mediator layer communication
* Updated: Single comdat thread of DeviceBase implementation


### v 0.9.8.5

---------------------------

* Updated: Secure channel handshake in DeviceBase
* Fixed: Bug in send thread due to disposal of send contexts
* Updated: Send thread usage for mediator layer broadcast socket with WinSock
* Added: Send thread usage for mediator layer broadcast socket


### v 0.9.8.4

---------------------------

* Updated: Mediator client send thread for winsock
* Added: Mediator client send thread
* Updated: Mediator client response handling with a map for multiple concurrent requests
* Updated: Timeouts at Mediator client layer
* Fixed: Mediator server client thread assignment
* Changed: Win32 Crypt layer using separate AES enc/dec lock 
* Fixed: UdpSocket non-block option of Core
* Added: Lock to list access in Echo.Bot and Simple.Console
* Updated: Threads.cpp for EnvLock and EnvSignal
* Fixed: Release crypt and reinitialize in Core


### v 0.9.8.3

---------------------------

* Added: Mediator server clients thread pool (experimental stage 1)
* Updated: Mediator client listener receive handling
* Added: Mediator server send thread using packs of contexts (experimental)
* Updated: Mediator protocol to V7
* Updated: Mediator client listener to handle multiple inner messages wrapped into an outer encrypted container
* Added: Mediator server send thread usage (experimental stage 1)
* Updated: Mediator server stunt lock usage
* Updated: Mediator server RemoveDevice lock usage
* Added: Mediator server send thread implementation
* Moved: Queue.Vector to Common (for Mediator server)
* Fixed: Bug in Mediator server loading of device mappings
* Updated: Stunt for Android platforms
* Updated: Stunt when concurrent direct connects are ongoing


### v 0.9.8.2

---------------------------

* Updated: Handleshort message at Mediator server
* Updated: Stunt for mult-homed hosts
* Added: Keep connected devices alive if they vanished from device lists
* Updated: Shutdown of sockets (set non-block)
* Added: Couting and waiting for stunts/stuns
* Updated: Disposal of Mediator client layer
* Updated: Mediator protocol version to V6 (using sequence number for response requests)
* Added: SetAppShutdown to Environs API to perform a quick shutdown
* Added: NoBlock-option to StopNetLayer
* Updated: Usage of timeout in Mediator communication
* Added: Lock to allocation of sockets in DeviceBase (i.e. udp socket race condition)
* Updated: Closing of udp socket of DeviceBase
* Added: Socket trace logs
* Reduced: Object size of DeviceBase


### v 0.9.8.1

---------------------------

* Updated: Stunt progress notifications
* Changed: Stunt thread from select to poll
* Changed: Main communication to comdat channel
* Fixed: Sync of broadcast update counter in mediator layer
* Fixed: Bug in Devicelist update of apparea in mediator layer
* Updated: Mediator server notify thread to check send ready before going into blocking call
* Changed: Naming of main to interact and bulk to comDat channel
* Updated: Mediator client mediator list reload (if list has been cleared)
* Updated: VanishedDeviceWatcher in Mediator layer
* Added: Using Winsock in Core
* Changed: Core listener to single thread instance
* Added: Logging of failed sends to derive alive status of clients in Mediator server
* Updated: CloseListener in DeviceBase
* Updated: DevicesCacheRebuild in Mediator client layer
* Added: Alivethread suspend phase before reconnect to mediator server
* Reworked: Stunt queue in Mediator client and server
* Updated: Mediator server thread usage
* Added: handleID to Threads.cpp for Win32
* Added: Check for disposal in DeviceList updater of MediatorClient
* Updated: PreDispose (WinSock) in cli layer
* Fixed: Sync-bug in Android DeviceList
* Reworked: Restart of Broadcast thread (in error cases)


### v 0.9.8.0

---------------------------

* Updated: ThreadSync and EnvThread and EnvLock
* Reworked: Closing of Core and DeviceBase socket listeners
* Reworked: Closing of Core socket listeners
* Fix: Broadcast/Alive thread restart (while errornous stop/start cycle by user code)
* Updated: Disposal of Environs and DeviceBase
* Updated: Stunt handshake to finish in less than 30 seconds. Otherwise invoke fail.
* Added: Mutual abort signal to DeviceBase communication
* Reworked: Socket shutdown/close procedure
* Fixed: Bug with enqueue of spare socket renewal during stunt
* Updated: Stunt code using single thread and non-blocking sockets
* Updated: Lock usage in MediatorRegistrator
* Updated: Usage of autoreleasepool in OSX ChatApp
* Updated: Handling of error case in starting of the Core
* Updated: BulkListener to use non-blocking socket
* Fixed: Cleanup in SaveToStorage (if something went wrong)
* Updated: Start/Stop of Environs cli instance (wait for all UI tasks to finish)
* Reworked: DeviceList notifications
* Fixed: Bug in DeviceList (ListUpdater and DeviceFlagsSync)
* Disabled: Ack notification for sent messages
* Updated: Queue.Vector compression approach
* Updated: Disposal of DeviceInstance when Environs instance is disposed
* Added: VectorListSP class
* Updated: Queue.Vector class (ring buffer), added Queue.Vector template class
* Added: Queue.Vector class (ring buffer)
* Changed: MessageInstance storage in DeviceIntance to a Queue
* Added: Restart of Alive thread (if something unexpected happened)
* Added: Restart of Broadcast thread (if something unexpected happened)
* Updated: Messages/Fileslist retrieval API in DeviceInstance
* Updated: MessageInstance creation in DeviceInstance
* Updated: Usage and comparison of same apparea flag in DeviceInfo
* Updated: Reloading of device list from Mediator in error cases
* Updated: DeviceBase connect check in Stunt request
* Removed: broadcastUpdateLock in MediatorClient
* Added: SetAllowConnectDefault () / GetAllowConnectDefault () to Environs
* Added: SetAllowConnect () / GetAllowConnect () to DeviceInstance
* Updated: UdpPack handling
* Updated: DeviceInstance - Receive/ReceiveBuffer/ReceiveData


### v 0.9.7.5

---------------------------

* Added: const ref to observer CPP API
* Fixed: Bug in AsyncWorker starting send threads
* Fixed: Cache app/area name in CLI layer
* Added: DeviceInstance - Receive/ReceiveBuffer/ReceiveData
* Updated: OpenSSL cleanup
* Updated: SendDeviceConfig in DeviceBase
* Added: StaticDisposer to CPP API EnvironsLoader
* Updated: Init of allocated memory for leak checks
* Updated: refactorMallocbuffer in DeviceBase
* Fixed: Member init bug in AsyncWorker
* Updated: DeviceInstance.toString()
* Updated: Disposal of devicelist updater threads in platform layer (cpp)
* Changed: SensorInput handlers to include all other udp data types


### v 0.9.7.4

---------------------------

* Updated: Device flags sync to AsyncWorker
* Fixed: Bug in Dispose sends from thread pool
* Added: Native ready to device flags
* Fixed: Device flags sync for udp syncs
* Updated: Device flags sync using backpropagation


### v 0.9.7.3

---------------------------

* Updated: Device flags broadcast sync (back propagation)
* Fixed: WorkerSend falsely reset envSPs (fix to evaluate)
* Updated: Device flags sync through Mediator servers
* Added: Connected indicator to ChatApp
* Updated: Core.Stop and transition lock
* Fixed: Bug in Update of device flags (AsyncWorker)
* Fixed: Bug in DeviceBase HandleStringMessage
* Added: No encryption into handshake
* Updated: ChatApp message handling
* Added: Usage of MSG_NOSIGNAL 
* Updated: ChatApp Windows


### v 0.9.7.2

---------------------------

* Updated: Stun connector
* Reworked: Threads.cpp Run-method to consider timeout throughout all wait calls


### v 0.9.7.1

---------------------------

* Updated: Short message handshake between devices
* Changed: In Android layer, opt() calls using jbyteArray instead of NewUTFString
* Renamed: ResetIdentKeys to ResetCryptLayer
* Updated: NotifyAppContextChanged of cli layer to trigger property changed name with given number
* Added: Caching of DeviceLists for faster access (CPP API)
* Added: Linear acc sensor read to Android layer
* Added: Mediator protocol version to broadcast identifier
* Updated: Mediator STUN protocol with flat and compressed apparea
* Updated: GetDeviceList from Mediator to V5 protocol
* Updated: Short messages between devices to version 3 protocol
* Updated: iOSX projects
* Updated: DeviceList handling if Mediator server is not available
* Added: SensorData callback handling to Android layer
* Updated: Alloc/Dealloc of global heap resource (instancesSP)
* Updated: Concurrent and mutual connection requests of two devices at the same time in DeviceBase
* Changed: Broadcast and Mediator protocol version to 5
* Added: Populate platform object activation flag
* Added: BackNotify of DeviceFlags in case the platform object becomes active
* Updated: DeviceBase for disabled mediator services
* Added: Type MediatorStatusMsgExt for device updates
* Added: Broadcast deviceflags update
* Updated: Mediator server with deviceflags update handler
* Updated: Usage of internalFlags
* Disabled: Dispatching of messages without a registered DeviceInstanceNode
* Updated: SetDeviceFlags with PlatformReady
* Added: SetDeviceFlags to Environs object
* Updated: DetectPlatform of linux layer
* Updated: iOSX header to use @import for automatic linking to required frameworks
* Updated: Check for max sends in queue of AsyncWorker
* Fixed: Bug in Echo.Bot
* Reworked: AliveThread
* Added: ChatApp2 iOS
* Updated: DisposeDevice in Devices.cpp (see Note)
* Updated: AsyncWorker threadpool to 32 for display devices, 12 otherwise
* Updated: Windows projects to use SharedAssemblyInfo.cs
* Updated: Log output in .net examples
* Added/Updated: ChatApp Windows using non-UIAdapter DeviceLists, ChatApp2 Windows using UIAdapter DeviceLists
* Added: Guard to DeviceBase listeners
* Changed: GetDevicesNoClone to GetDevicesSource
* Added: Echo.Bot example
* Added: Environs.QueryMediatorLogonCommandLine for command line mode
* Changed: GetDevices() of DeviceList to return a cloned list
* Fixed: DeviceList observer add/remove if list is not UIAdapter for cpp/cli
* Updated: Simple.Console example windows
* Added: Instance option SetConnectAllowFromAll/GetConnectAllowFromAll
* Added: GetUseLogToStdout to Environs object
* Updated: Simple.Console example
* Added: AddObserverFor.. to cli Environs object
* Added: Simple.Console.CS example for windows .NET (using Environs.NoUI)
* Added: Windows .NET Environs.NoUI lib using platform toolkit v90
* Added: Simple.Console.CPP to OSX examples
* Added: Support for Raspberry PI Jessie and Wheezy
* Added: SetUseLogToStdout for enable/disable log to stdout. Linux default is disabled. Otherwise enabled.
* Updated: Usage of type SensorFrame in SensorObserver
* Added: Simple.Console.CPP examples


### v 0.9.7.0

---------------------------

* Fixed: iOS locationManager updates
* Updated: Threadpool size of AsyncWorker increased to 8
* Disabled: Threadname assignment
* Updated: LoadNetworks for Android
* Disabled: Stun limits at MediatorDaemon
* Updated: Stunt process
* Updated: iOSX SensorData examples
* Added: Location sensor reading to Win32 platform layer
* Updated: Sensor code for Android native layer
* Updated: Sensor code for iOS layer
* Updated: enum usage in Observers
* Changed: MessageObserver and DataObserver to use enums as flags
* Changed: DeviceObserver to use DeviceInfoFlag as type for changedFlags (CPP/CLI)
* Added: Annotations to android api layer
* Added: Crypt layer init for Android Marshmallow
* Fixed: Bug in MediatorDaemon and ip bann
* Added: Notify enumerations and j2c to create Notify class/namespace
* Updated: Usage of enums (Android layer)
* Added: j2c Export of enums to enum source files from Types.java (Android layer)
* Updated: Usage of enums (CPP/CLI layer)
* Moved: enums from Environs/Types class to environs namespace (CPP/CLI)
* Updated: Usage of enums (iOSX layer)


### v 0.9.6.10

---------------------------

* Fixed: Bug in UdpListener of DeviceBase
* Updated: Usage of enums (iOSX layer)
* Reworked: j2c and enum usage in platform layer


### v 0.9.6.9

---------------------------

* Fixed: Bug in Sensor handling
* Added: IsSetSensorEventSending(type) to DeviceInstace to query status of sensor event sending
* Added: Enumeration Environs.SensorType (as a replacement for ENVIRONS_SENSOR_TYPE_)
* Updated: j2c with generation of arrays
* Replaced: hcmbg.png with jpeg for Android examples
* Updated: Log output of Android layer
* Updated: Usage of SendPing
* Updated: Utils::logLevel in cpp/cli layer
* Added: SendPing to DeviceBase (test of channels)
* Updated: Core.Start
* Updated: AliveThread (Close if no network and restart if network becomes available)
* Updated: NETWORK_CONNECTION_ declarations
* Fixed: Bug in Init of ObjectAPI at cpp/cli layer


### v 0.9.6.8

---------------------------

* Updated: Generate of random deviceID if another device has claimed the identity
* Fixed: GetTickCount of Android native layer
* Updated: Disposal of SensorSender
* Updated: Mediator daemon check for deviceName for busy slot detection
* Updated: Mediator daemons client thread when a identity is already registered by someone
* Added: GetLocation() to environs object


### v 0.9.6.7

---------------------------

* Added: Logroll to environs logfile
* Updated: Environs.Sensors.cpp (disposal of receivers)
* Added: iOS project for SensorData
* Updated: SensorData streaming (tcp/udp)
* Reworked: Sensor management
* Moved: Sensor related code to class Sensors in Android layer
* Updated: SensorData Windows to identify itself as LocationNode
* Removed: Unused references from cli/cpp dlls
* Added: CLI/CPP NoUI projects
* Added: SensorData Windows with orientation oszigraphs 
* Updated: dirent for cli/cpp to use managed functions
* Updated: iOS sensor event handling
* Added: SensorData OSX with orientation oszigraphs 
* Added: SetIsLocationNode() to Environs object
* Added: Environs.Platforms.LocationNode_Flag
* Updated: Thread disposal in Threads.cpp
* Updated: API calls that sends network data to keep an SP of its environs instance
* Added: Status stop begin to environsState
* Added: Macro to disable timestring in log output
* Updated: Sensor layer global state init/dealloc
* Updated: Mangement of Environs native instances
* Updated: Disposal of Environs native instances
* Updated: Stun/Stunt min wait time between notifications


### v 0.9.6.6

---------------------------

* Updated: Usage of AsyncWorker.DisposeSends
* Updated: Disposal / Stop of Environs object (.NET PL)
* Fix: Crash during disposal of native layer in PortalInstance::portals (static) (To verify)
* Added: SetTitle after environs start to ChatApp
* Fixed: Bug when saving an incoming message to storage
* Added: SetNetworkConnectTimeout to Environs object
* Updated: Android ChatApp threadpool for sending
* Updated: Invoke frequency of GC
* Updated: Disposal of DeviceBase
* Added: Check for Environs status before accepting any network interactions by devices
* Fixed: Device list sending wrong update notification (BC/M) in mediator layer


### v 0.9.6.5

---------------------------

* Fixed: Consistent update of cached DeviceLists in mediator layer immediately after start of network layer
* Updated: keyMonitor for OSX layer (Moved to Environs.OSX.mm)
* Updated: DevicesMediatorReload to empty cache if no mediator is available and functioning
* Fixed: GetEnvironsTickCount for Android layer (wrong calculation resulted in Heartbeat timings and disconnects from Mediator server)
* Updated: Disposal by GC
* Updated: Stunt (check for environs disposal)


### v 0.9.6.4

---------------------------

* Updated: Stopping of network layer during disposal of environs
* Updated: socket calls in mediator client layer
* Updated: socket calls in mediator daemon
* Reworked: Shutdown of platform layer and native layer
* Added: Waiting for GC thread on disposal of Core (RemoveAllDevices would otherwise not guarantee that all devices have been removed)
* Added: Macros for socket buffers adapt and set to nonblock of client sockets at Mediator daemon
* Changed: CommitSend of MediatorClient to lock only if a response is expected
* Reworked: MediatorListener of client layer to dispatch longer lasting ops to async worker
* Updated: Stunt disposal in DeviceBase
* Updated: Increased stunt request timings
* Updated: Stunt process
* Updated: Mediator service stunt register request frequency
* Updated: Log output
* Updated: Connect with timeout in mediator layer
* Updated: Socket receive timeout in mediator layer
* Updated: Windows projects and solutions
* Updated: Build scripts to use MSBuild.exe


### v 0.9.6.3

---------------------------

* Fixed: Synchronous update of devicelist Android layer
* Added: Threadpool for send message in ChatApp Android
* Added: Threadpool for send message in ChatApp Windows
* Added: LaunchImages and AppIcon to iOS MediaBrowser


### v 0.9.6.2

---------------------------

* Updated: ConnectAsync / ReceiveAsync
* Fixed: Bug in Devices.cpp (Check after disposal of a device)
* Fixed: GetDeviceID from Mediator
* Fixed: Crashes of OSX apps due to nil ssid
* Updated: Cancel of Stunt in case Environs is stopping
* Updated: Stopping of NotificationQueue in case the UI thread called Stop
* Added: Refresh DeviceLists after Environs has started
* Updated: ChatApp OSX (crash while retrieving profileText)
* Changed: AsyncWorker send thread to thread pool
* Added: async flag to DeviceInstance.SendMessage
* Fixed: Bug with Win32 SendFile where error log tries to output ws instead of s
* Fixed: Bug in Mediator layer if multiple (virtual) network interfaces are available or multi-homed hosts
* Updated: Log layer
* Updated: CPP CLI property notification in DeviceInstance
* Updated: GC thread


### v 0.9.6.1

---------------------------

* Fixed: Bug in DeviceBase disposal
* Updated: Loading of background image in ChatApp.CPP and WindowWin32 (CPP API Windows)
* Added: LoadPicture to Environs object (CPP API using GDI+)
* Updated: Notification in DeviceInstance (.NET layer)
* Updated: projects to use hcmbg.jpg instead of png


### v 0.9.6

---------------------------

* Updated: PortalInstance to use nativeID throughout all layers (nativeID is part of the keys)
* Updated: Disposal of DeviceBase and stunts
* Reworked: Starting of Mediator listener at client layer
* Reworked: GC DeviceBase handling
* Added: timestamp to log output
* Changed: SendMessageN to take a memory pointer (instead of string) to avoid platform conversion issues
* Reworked: MediatorListener in Mediator client layer
* Updated: MediatorDaemon WatchDog client alive check
* Updated: ChatApp OSX protocol (request text before image)
* Updated: SetDebug/GetDebug of Environs object
* Added: KeyEvent handler to osx layer
* Updated: usage of weak pointer in DeviceBase
* Fixed: Bug in message notifications (using correct source type)
* Added: (Fix for mediator responses) Padding to DeviceInfo
* Added: Safe dispose of session clients in MediatorDaemon
* Fixed: Bug in MediatorDaemon where mssing areaMaps lead to failing of registration of new clients
* Updated: DeviceList notifications using objID
* Updated: Usage of objID in native layer and platform layer for DeviceInstance
* Updated: Observer notification handling in platform layer
* Added: sp/wp between DeviceBase and DeviceInstanceNode
* Added: objID to DeviceInstance objects
* Added: Wait loop for CommitSend if large amount of short messages are delivered
* Updated: Use of lock callback in crypt layer (CryptoGetThreadID)
* Updated: Detection of natStat moved to registering at Mediator service
* Updated: ReceiveAsync and ReceiveOneMessage in DeviceBase / MediatorClient
* Fixed: SecureChannelAuth in Mediator layer if message is transfered multipart
* Fixed: SecureChannelProvide / SecureChannelEstablish if message is transfered multipart 
* Fixed: Handshake of short message if message is transfered multipart
* Added: Use of lock callback in crypt layer
* Updated: Thread starting in native layer
* Updated: MediatorDaemon protocol for shortmessages
* Renamed: GetDeviceList to CreateDeviceList (Environs object)
* Added: async flag to Environs object (platform layer)
* Added: Using host part of IP for deviceID generation (if its set to 0)
* Updated: Disposal of FileInstance / MessageInstance of DeviceInstance
* Fixed: Bug in DeviceList updating where device updates leak temporary device instances
* Added: ERR_remove_thread_state to DynLib.Crypto
* Changed: info_ of DeviceInfo to dynamic allocated
* Updated: Macro usage for platform layers
* Updated: iOSX ChatApp (disposal of ChatUser)
* Added: Find default gateway device to Android / OSX native
* Added: Retry to STUNT EstablishRequest
* Updated: ChatApp protocol to synchronize with the according remote ChatUser object after init
* Added: USE_LOCAL_MEDIATOR_CACHE_PARAMS to use cached mediator device list for params
* Updated: DeviceBase Connection state sync with SendMessage
* Updated: GetThreadInstance in MD
* Updated: HandleShortMessage in MD
* Reworked: Windows ChatUser protocol
* Reworked: STUN and lock usage
* Reworked: STUNT and spare socket registration process


### v 0.9.5.2

---------------------------

* Fixed: Network interface detection on windows platforms
* Reworked: ChatApp OSX protocol
* Updated: ClearMessages / ClearStorage using async flag of DeviceInstance objects


### v 0.9.5

---------------------------

* Moved: Disable linger to closesocket
* Updated: DeviceBase Connector to use external ip if the internal ip could not be determined by the mediator layer
* Updated: Notification loop handling in platform layers
* Changed: Removed NewStringUTF from Android layer with externally originating strings. NewStringUTF crashes unrecoverable on some devices and on some not ...
* Added: SetMessagesSubscription/GetMessagesSubscription to Mediator layer
* Added: SetMediatorNotificationSubscription/GetMediatorNotificationSubscription to Mediator layer
* Added: Size check in TcpListener/BulkListener of DeviceBase before refactoring the heap located receive buffer
* Changed: Workaround for BitmapFactory.decodeByteArray for Android ChatApp
* Updated: synchronized usage in threads for Android layer
* Disabled: BitmapFactory.decodeByteArray for Android ChatApp (creates high cpu load that never drops)
* Added: ERR_remove_state to cleanup crypto resources on error
* Reworked: lock usage in Android Environs layer
* Fixed: Crash in DeviceInstance due to possible init of std::string with nullptr
* Added: ENABLE_DONT_LINGER_SOCKETS to native layer
* Reworked: MessageThread of Android ChatApp
* Updated: Threading in Android layer DeviceList/DeviceInstance updates/notifications
* Added: DeviceInstance notifier thread to android layer
* Added: DisposeList to platform layer DeviceList object
* Added: Lock to spare sockets container
* Updated: ReplaceThreadSocket in MediatorDaemon
* Updated: Disposal of ThreadInstances on shutdown of MediatorDaemon
* Updated: Handling of spare sockets in MediatorDaemon
* Updated: Thread detaching in platform layer
* Fixed: Detaching of ClientThreads for spare socket registrations
* Fixed: EnvironsTester (Send tests)
* Added: Assertion of max bulk send size in DeviceBase
* Added: Try/Catch to SaveToStorage in DeviceBase
* Added: PreDispose and PrepareRemovalOfAllDevices to native layer (Cancel all device activities in advance before we actually wait for them to finish one after another)
* Updated: Core Start
* Fixed: Crash due to disposed DI in Android ChatApp
* Fixed: Access of disposed DeviceInstance in Android ChatApp
* Fixed: ParseStorage in Android platform layer
* Fixed: wchar bug in SendFile of DeviceBase
* Added: Check for outdated project values in MediatorDaemon (WatchDog)
* Reworked: DeviceList Remove notifications
* Changed: PlatformDisposal of DeviceInstance moved to DeviceList notifier thread
* Updated: CPP CLI layer for DI notifier thread changes
* Changed: CPP layer -  Run a DeviceInstance notifier thread for each Environs instance
* Added: Option s to toggle stdout MD
* Updated: iOSX ARC disposal of DeviceInstance
* Updated: iOSX ChatApp notification handling
* Updated: DeviceInstance update notification control using static flag (DeviceInstance.notifyPropertyChangedDefault)
* Added: Fallback to LoadNetworks for Windows in Mediator base layer
* Updated: CPP API / CLI layer to provide non-UI device lists


### v 0.9.4

---------------------------

* Release 0.9.4
* Updated: DeviceBase SendMessage to route through Mediator if connection is still ongoing
* Updated: MediatorDaemon NotifyClients
* Updated: Environs crypt layer
* Fixed: Bug in init of MessageInstance / FileInstance
* Fixed: Bug in broadcast message handling of MediatorClient
* Updated: Generation of correct broadcast message at Start of an Environs instance
* Updated: PlatformDispose of MessageInstance / FileInstance (.NET)
* Updated: PlatformDispose of MessageInstance / FileInstance (iOSX)
* Updated: Android Environs build gradle to use support library version 22
* Updated: ChatApp Windows (invalidate messageslist if the related device vanished)
* Added: Disable DISABLE_MEM_CHECKS to build opts
* Added: Lock to UpdateAppEnv of EnvironsNative
* Fixed: Lock bug in AsyncWorker
* Updated: AsyncWorker PushSend
* Added: Toggle Acceptor of Mediator Daemon (j)
* Added: Receive of certificates (during handshake) in chunks
* Fixed: DeviceBase check for invalid certificate (size)
* Updated: ChatUser Windows
* Updated: Notifications/AsyncWorker Pop
* Updated: Notifications/AsyncWorker thread initialization
* Updated: Management of platform ARC references (iOSX)
* Updated: Environs.Build.Opts.h
* Updated: CPP CLI log layer with StringBuilder
* Updated: DeviceInstance platform layer to dimiss adding of messages/files if DI is disposed (memory leak)
* Fixed: Username detection in iOS ChatApp
* Fixed: ARC issue of DeviceInstance (using autoreleasepool for vanished/appeared list)
* Updated: Tcp/Bulk listeners in DeviceBase
* Updated: Lock usage in Mediator client layer
* Updated: Windows Mediator local network adapter detection
* Updated: DeviceBase connection process (do not dispose device if GetDeviceForHandshake fails in case of an ongoing connect by another entity)
* Updated: ThreadSync in DeviceBase
* Updated: Platform disposal of ARC objects
* Added: Allow windowless usage of Environs
* Fixed: ThreadStart of ConnectorThread
* Added: DeviceInstance reference to .NET SensorFrame
* Test: Allow concurrent Stunt connections
* Updated: iOSX ARC SP


### v 0.9.3

---------------------------

* Release: 0.9.3
* Replaced: NULL_ptr with nill
* Updated: Usage of snprintf (iOSX, Mediator)
* Replaced: sprintf_s with snprintf (_snprintf for vs < 2015)
* Removed: Mediator daemon support for protocol version <= 2
* Fixed: Message parsing in windows ChatApp
* Updated: Disposal of DeviceBase
* Updated: ThreadSync cpp cli
* Added: ThreadSync in NotificationQueue
* Updated: ThreadSync in AsyncWorker
* Updated: Core and ThreadSync
* Added: ThreadSync areWeTheThread
* Updated: Added Mediator response timeout 3 min. (in case the provided response buffer is larger than 512 bytes)
* Added: Assertion of Mediator returned device list strings
* Updated: Stunt/Stun with ThreadSync
* Updated: Usage of ThreadSync in Stunt (Acceptor)
* Updated: Usage of ThreadSync in Core
* Updated: Usage of ThreadSync in DeviceBase
* Added: ThreadSync class
* Removed: S2 and S2.Xna from vs2010 dev solution
* Updated: STUN log
* Added: PrepareStorage to prepare storage if messages arrive (through mediator) while connection is about to be established
* Changed: Moved UpdateDeviceStatus outside of parent lock section
* Updated: log output
* Updated: Core with threadstates for threads
* Updated: DeviceBase with threadstates for listeners


* Release: 0.9.2
* Fixed: Bug in WaitForDeviceDeletion of Devices
* Reworked: DeviceList updater and lock usage in MediatorClient
* Improved: DeviceList updater in Mediator client layer
* Updated: Lock logs with threadID
* Updated: StuntRequest with pointer to deviceStatus (disposable by static SendMessage connects)
* Updated: Starting of ConnectorThread in DeviceBase to make sure the object instance is not released until the thread has started
* Enabled: Lockfree access to sockets
* Updated: Lock logging
* Updated: Stunt disposal
* Fixed: Disposal bug in DeviceBase
* Reworked: Device management and SP
* Updated: Use of SP in DeviceBase and device management
* Changed: Stun/T to use SP
* Reworked: DisposeThread / DetachThread
* Updated: LoadNetworks / ReleaseNetowrks in Mediator base layer
* Updated: Disposal of DeviceBase / DevicePlatform
* Changed: Using strlcpy/strlcat instead of strcpy_s/strcat_s
* Moved: CloseConnectorThread to DeviceBase
* Reworked: LoadNetworks/AddNetwork in Mediator base layer
* Fixed: Dispose issue of DeviceController (CloseListener in derived classes)
* Updated: Environs.cpp to check the api instance before accessing

* Updated: MD lockfree usage of sockets
* Added: MD logroll if file is larger than 100MB
* Updated: DeviceList acquire lock before dispatching to UI thread
* Changed: Moved device->Update outside the UI thread in DeviceList
* Added: MediatorDaemon reopen logfile regularly by watchdog
* Added: Mediator vs2013 solution
* Reworked: Log subsystem
* Updated: Android DeviceList
* Added: UI threaded devicelist update
* Changed: Device.Marker.cpp and Finger.Input to Input.Handler.cpp
* Updated: Added lock section to device connects in DeviceBase
* Updated: Marker handling for TUIO injected events
* Updated: Detect real physical PixelSense device
* Fixed: Mobile settings load (global, instance)
* Added: Stunt/Stun max try settings to native object

* Release: 0.9.1
* Fixed: Bug in DeviceBase (HandshakeShortMessage), Buffer overrun in log subsystem
* Fixed: Bug in MediatorDaemon (HandleSTUNTRequest)
* Updated: Comment to StorageLock()
* Updated: StorageLock() to DeviceInstance for safely accessing MessageList/FileList
* Fixed: Some rare conditions of uninitialized variables
* Fixed: Uninitialized variable in MediatorDaemon
* Updated: PixelSense DeviceMarker in native layer
* Fixed: Android layer portal lookup (used the wrong key format)
* Updated: Android PortalInstance notifier to use lock
* Updated: MediatorDaemon Disposal of ClientThread resources
* Updated: MediatorDaemon to check connectTime within WatchDog
* Updated: Init of hEnvirons in PortalInfo
* Updated: PortalInfo reference on PortalInstance (iosx)
* Updated: PortalInfo reference on PortalInstance
* Updated: Disposal of AsyncWorker SendThread
* Added: Check Mediator filter level for shortmessages in DeviceBase.cpp
* Added: Assert of certificate buffer size received by client response (DeviceBase)
* Updated: MediatorDaemon with DeviceInfoNode
* Updated: Usage of deviceSP in MediatorDaemon
* Fixed: iOS ChatApp (static loginUsername reference)
* Updated: Android layer to secure observer management with locks
* Added: Lock observer management in DeviceInstance, DeviceList, Environs
* Updated: Relinking of vanished devices in Mediator.cpp
* Fixed: Bug in Windows ChatApp
* Fixed: Bug in iosx ChatApp
* Updated: ChatApp project with EnvironsTester (public projects)
* Added: Dispose AsyncWorker send packets from send queue when a device gets disposed
* Added: Usage of SP for AsyncConnect and AsyncReceive (MediatorClient)
* Updated: StartN to start only if Environs is in Stopped state
* Added: Assertion in BuildKey (MediatorClient) to handle wrongly returned Mediator responses
* Fixed: Show login dialog only if Environs is started and anonymous logon is disabled
* Fixed: Removal of vanished devices (DeviceList) in Android layer
* Updated: Disposal of NotificationQueue and AsyncWorker
* Changed: Device management instances to SP
* Fixed: Pass correct hEnvirons to Trackers
* Reduced: Stunt max tries to 10
* Changed: Mediator client instance to SP
* Updated: Mediator client access while disposing
* Updated: Concurrent access to mediator (while disposing)
* Updated: NotifyDatasetChanged handling of Android layer for BaseAdapters
* Added: ViewGenerator to ClipPort
* Updated: ViewGenerator interface (Android layer)
* Changed: GetDeviceListObj in Environs.h to GetDeviceList
* Added: CPP CLI v140 project and adopted solutions to v140
* Updated: windows solutions to conditionally depend on v100/v120 projects
* Updated: AliveThread sending a helo to help vanished devices watcher (duration 120000 ms)
* Added: Vanished devices watcher to base Mediator layer (for broadcast found devices)
* Fixed: Writing of file chunks by DeviceBase
* Updated: FileInstance created/available flag and progress notifications
* Changed: Native calls are appended an N
* Updated: GetFile in DeviceInstance (.net managed)
* Changed: Login.Dialog.h to Login.Dialog.Win32

* Changed: Removed support for Surface 1 tabletop

* Updated: Android projects (Observer contexts)
* Added: Macro for platform freeing of memory
* Updated: Notification context usage in observer signatures
* Updated: CPP/CLI layer
* Refactored: Namespace in android, wpf layer and examples
* Changed: Default namespace from hcm.environs to environs
* Updated: win32 use of sync variables
* Changed: IEnvirons.Dispose to interlocked mechs (beta)
* Refactored: Namespaces and Interface names
* Updated: .NET lib loader
* Updated: Windows examples
* Updated: App shutdown in WindowsWin32 example
* Added: OSX Alert dialog in case of missing library
* Updated: OSX Environs.Loader to include working folder of the client app

* Release: 0.9.0
* Updated: Log format
* Updated: Android SensorData example
* Updated: Dispose of PortalInstance in iOSX layer
* Updated: Android example projects (new Observer interfaces)
* Updated: .net layer with data/message observer flags
* Updated: Data/Message observer interface with flags
* Updated: Environs status Start/Stop Created/Released (Environs.Lib.cpp)
* Updated: Android / iOSX / CPP API to allow different settings for multiple instances with different app/area names
* Added: ChatUser to ChatApp.cpp windows
* Changed: Windows ChatApp.CPP to ChatApp
* Adapted: Android example projects to new API
* Updated: Settings keys and storage handling (mobile devices)
* Updated: Loading/Saving of settings (display devices)
* Updated: LoadConfig/SaveConfig
* Updated: area app mappings to configuration file
* Added: EnvironsNative object as static initializer
* Updated: Major changes to support hEnvirons handles
* Updated: .NET LibLoader
* Updated: Environs iOSX API using CPP API
* Updated: CPP API Observer Interfaces
* Fixed: Anonymous and U/P logon handling within mediator layer
* Added: UUID generation for Linux platforms
* Changed: DeviceInstance Notifier thread now static (.NET)
* Changed: UpdateMessage, Data, etc.. in DeviceInstance
* Updated: DeviceList Interface
* Changed: Notification thread starts with the Mediator service
* Added: Persistent storage of latest assigned identifier for area/app
* Changed: Callback pointer type from long to void * (in Environs.Lib.API) due to 32/64 bit platforms
* Added: Threaded notifications of DeviceList Android, iOSX
* Added: Threaded notifications of DeviceInstance WPF
* Added: Threaded notifications of DeviceInstance Android
* Changed: Observer interfaces in DeviceInstance iOSX seperated to different classes
* Added: Threaded notifications of DeviceInstance iOSX
* Updated: Observer API documentation
* Updated: ChatApp.CPP Winodws (message observer)
* Updated: ChatApp.CPP Winodws (chat history)
* Updated: RemoveObserver from device for ChatApp (Windows, Android)
* Updated: iOSX CPP API examples
* Updated: Environs.Loader
* Added: ChatApp.CPP to windows examples
* Updated: WindowWin32 example
* Updated: iOSX projects to CPP API changes
* Updated: RemoteTouch.OSX.CPP and iOSX PortalInstance implemetation
* Updated: iOSX CPP API reference management
* Added: iOSX CPP API Environs Observer
* Changed: ChangeObserver to DeviceObserver
* Updated: iOSX CPP API
* Updated: Usage of Interfaces in CPP API
* Added: Macros for sp instance management in CPP API
* Changed: Device.Instance files/message to smart pointer managed
* Updated: CPP API
* Updated: Usage of MutexLock / MutexUnlock
* Added: CPP API xcode projects to OSX examples
* Updated: iOSX/Android projects use of GetDeviceList
* Updated: Windows projects use of GetDeviceList


* Added: GetDeviceList to .NET/Java Environs
* Added: x86 build type to WindowWin32 project
* Updated: Usage of MutexLock/Unlock
* Fixed: Mutex init but in Device.Base.cpp
* Added: First wrapping code for iOSX (using CPP API)
* Fixed: Notification handling in CPP API
* Updated: Notification handling using Contexts (Notify.Context.h)
* Updated: Disposal of CPP API objects
* Added: Platform init for CPP API
* Added: Environs.Sensors to display platform layers
* Added: Observers for CPP API
* Added: Interfaces for CPP API
* Changed: Environs Interface to IEnvirons
* Added: win32 Libloader for CPP API
* Added: win32 linker Environs.lib for 32/64 bit
* Updated: Environs.h for osx/ios
* Updated: Environs.h for windows/linux
* Updated: ENVIRONS_LIB_API for non-windows systems
* Added: Factory method to Environs.h ( CreateInstance () )
* Added: Minimalistic dirent wrapper around win32 findfile
* Updated: CPP API Portal.Instance.h/.cpp, Message.Instance.h/.cpp, File.Instance.h/.cpp
* Updated: CPP API Device.Instance.cpp
* Added: cflag -std=gnu++11 to Linux Makefile
* Updated: Windows solutions with environs::lib header/source files
* Added: CPP Device.Instance.h / .cpp and Device.List.h / .cpp
* Added: RemoteTouch.CPP (OSX) that uses environs::lib API
* Added: CPP Environs API Environs.h/Environs.cpp
* Renamed: Environs.h/.cpp to Environs.Obj.h/.cpp
* Updated: Windows Environs solutions
* Added: SensorData projects to environs windows solutions

* Release: 0.8.8
* Updated: Working version of Winodws SensorData project
* Added: WPF UserControl OscilloscopeSimple (code from OSX View)
* Renamed: OSX SensorData project to SensorData.OSX (preparing for upcoming ios project)
* Added: Windows SensorData project
* Fixed: Windows RemoteTouch project (missing hcmbg.png in project file)

* Release: 0.8.7
* Updated: OSX SensorData Osci View
* Updated: UpdateValue of SensorOsziView.mm (OSX SensorData)

* Updated: OSX sensor observer implementation
* Added: OSX Osciloscope View for SensorData project
* Added: Project SensorData to OSX
* Changed: Name of SensorEmitter to SensorData
* Updated: iOS sensor support (gyroscope)
* Added: Gyroscope sensor to the set of supported sensors
* Added: SensorData receiver to native layer for all platforms
* Added: Sensor data notifications to display device native layer
* Updated: iOS sensor handling
* Fixed: Device.Instance class to include sensor handling only for iOS
* Added: Initial sensor handling in iOS native layer
* Updated: Version of sensor event transmissions
* Updated: Sensor handling in Android DeviceInstance
* Updated: Sensor handler in Device.Display.cpp
* Updated: Sensor queue handling in mobile native layer.
* Updated: Mediator Linux makefile (include paths).
* Fixed: Mediator OSX compile error due to missing openssl headers in OSX El Capitan.
* Added: Sensor handling thread to Environs.Sensors.h
* Updated: Sensor handling in native and Android layer.
* Added: Example project SensorEmitter.
* Test: updated android sdk.
* Updated: Sensor event handling. Added SetSensorEventSending to DeviceInstance and RAW API. Added SetSensorEventSending1 to native layer for mobiles.
* Changed: Android compiler to clang

* Release: 0.8.6
* Added: Linux build of Android binaries to CI yml.
* Fixed: download tuio helper
* Updated: Linux build.
* Added: Linux device platform controller.
* Update: Download a certain openssl header version in helper scripts.
* Test: openssl issue for X509_get_notBefore.
* Fixed: Include error in Device.Android.cpp
* Updated: Download helper files.
* Added: gitlab ci yml.

* Added: Anonymous logon to Mediator (reduced privileges to one application environment).
* Extended: User database and encryption.

* Updated: ReadMe.md
* Updated: Download helper scripts to Android directories.
* Changed: Androids folder renamed to Android.
* Changed: Support for Eclipsed dropped.

* Removed: Unused platforms from build selection of windows solutions
* Added: Skipping compile of code for some plugins when 3rd party headers are missing
* Added: Warning headers for 3rd party headers
* Added: Nickname and status message to settings of ChatApp
* Updataed: iOX ChatApp WatchOS
* Changed: iOS ChatApp Watch from Swift to Objective-C

* Added: devices member to DeviceList to get an array/list of the DeviceInstances in the list.
* Changed: name of Android library to libEnvirons.aar
* Added: Compile x86 for Android library.
* Added: Android wear app for ChatApp.
* Added: iOS ChatApp Watch with WatchOS extension (initial template).
* Updated: iOS xcode projects to link against libEnvirons.a in libs folder.

* Changed: Android encryption layer uses libcrypto if available.
* Added: dynamic access of openssl to Android native layer.
* Updated: dynamic accessing of openssl.
* Added: scale factor (Retina) for resolution and dimensions to OSX layer.
* Updated: PortalInfo objects.

* Release: 0.8.4
* Changed: Use of HW encoder/decoder by default on OSX.
* Updated: OSX HW h264 decoder (VTToolBox) to draw on given surfaces.
* Updated: iOS and OSX camera capture plugin to use the correct orientation depending on the type of camera.
* Added: Check for matching InterfaceTypes after creating an interface object.

* Updated: wpf layer portal to better adapt to choice of source cams.
* Updated: direct show camera capture and yuy2 converter.
* Added: YUY2 to portal data types.
* Added: Ignore a broadcast device if another one with the same identifiers has already been registered.
* Fixed: Bug in message parsing of ChatApp examples.
* Changed: Linking to openssl 0.9.8 on mac os (Mediator and Environs library).
* Added: Assinging of random deviceID if no mediator connection is available.
* Added: and changed stunt to include trying to connect private networks.

* Updated: Windows solutions and projects.
* Added: Windows CamPortal example.
* Added: Windows camera dshow capture plugin and osx camera capture plugin.
* Added: Windows NullRenderer plugin.
* Added: Drawing of portals to osx NSView/NSImageView.
* Added: OpenH264 decoder wrapper plugin for OSX.
* Fixed: Some bugs due to changed init procedure of portal generator states.
* Changed: Creation of portal generator worker stages. Initialization of stages now happens after successful connections.
* Changed: Encode processing of Encoder.Base.h to use preinitialized function pointer.

* Added: libDir to Environs object to allow library directory different from working directory.
* Fixed: Bug in determining working directory for OSX.
* Improved: Disposal of portalinstance objects.
* Added: Threaded context to PortalInstance Establish procedure to allow user/app to block the call as long as they might need to.
* Fixed: Bug in flushing of touch frames during disposal of the touchsource.
* Changed: TouchSource object responsibility (in iOS and Android layer).
* Added: properties to iOSX object APIs.
* Added: Wait barrier to Connect when in CALL_WAIT mode.
* Added: Portal notifcation NOTIFY_PORTAL_ESTABLISHED_RESOLUTION to signal that the stream resolution init has been received.
* Updated: EnvironsObserver for incoming portals.
* Added: Null-Interface (Render, Encoder) to windows plugins.
* Fixed: Bug in touch input adaptation for view resolutions.
* Added: User defined portal resolution in request packet.
* Fixed: Bug in update of rendercontext for max context workers of potal generators.
* Added: Argument for resolution divider to portal SetRenderView.
* Updated: Portal establish procedure
* Updated: CamPortal example

* Updated: to version 0.8.0
* Updated: Android gradle build files.
* Added: Waitable connect of devices.
* Fixed: Bug in device main channel communication.
* Changed: ProjectName to AreaName
* Changed: CALL_SYNC/ASYNC to CALL_WAIT/NOWAIT
* Updated: Checking of manifest.
* Updated: Code comments.
* Added: EnvironsObserver to examples.
* Added: HelloWorld and CamPortal examples.
* Updated: Code comments.

* Updated: for release 0.7.4
* Fixed: Bug in SendBufferInChunks ()
* Updated: Windows ChatApp example

* Updated: iOSX ChatApp examples
* Updated: Tutorial ChatApp examples

* Updated: for release 0.7.3
* Added: Example tutorial application ChatApp for iOS, OSX, Android, Windows
* Fixed: Bug in Handshake of Devices
* Fixed: Bug in MediatorListener of MediatorClient to receive multiple chunks of larger messages
* Fixed: Bug in Sending of Messages and Files in AsyncWorker

* Updated: for release 0.7.2
* Added: File and message history to DeviceInstance of iosx/android platform layer
* Added: File and message history to DeviceInstance of wpf platform layer
* Added: DeviceList notification contains old and new devices as array parameter
* Changed: DeviceObserver to ListObserver and ChangeObserver
* Changed: name of DeviceLists to DeviceList

* Updated: OSX project settings
* Updated: iOSX and Windows projects for release
* Updated: iosx layer and project files.
* Added: HCM background to android example apps
* Moved: GetPortalAutoStart() to DeviceInstance object (Android).
* Updated: iOSX API
* Fixed: Disconnects from Mediator due to inactivity of devices.
* Updated: Portal/Device API
* Addded: Max wait time for portal requests (Environs.MAX_PORTAL_REQUEST_WAIT_TIME_MS)
* Reimported repository
* Updated: Rework of device list updater thread.
* Updated: Heavy rework of devicelist management. Added BaseAdapter for devicelists to Android platform layer.
* Changed: Member id in DeviceInfo to deviceID
* Added: DeviceController subclasses for OSX / Windows display devices
* Added: Shortcut methods to the DeviceInfoObject (.h/.cpp) Object
* Added: Check of external IPs and subnet mask to determine whether the devices are in the same network
* Added: Platform integer to broadcast message and DeviceInfo struct. (COM protocol change. Breaks compatibility with earlier versions.)
* Updated: DeviceItem visual
* Added: DeviceItem visual for available devices within the object
* Updated: WPF device list management (introduced DeviceLists class)
* Added: Check whether touch contact is occluded by a touch visual (to be tested)
* Changed: portalDevice lock mechanism using interlock variables and a spin lock for release
* Added: getDeviceForPortal1 that queries a DeviceInfo for a given portalID
* Added: Threaded applicatin exit to Windows platform layer. Preventing hang on shutdown when a portal receiver is ongoing (that renders using the UI thread)
* Updated: Usage of portalDeviceID for start/stop/pause/etc. of portals
* Added: portalDeviceID as identifier that targets a particular portal of a particular device.
* Added: Acquire/Release/Usage of receiverID for fast receiving of portal units (platform layer portals)
* Fixed: Bug in flushing of gesture recognizers in case the whole touch frame drops (ios)
* Added: Gesture recognizer status constants
* Renamed: StreamReceiver.java to PortalReceiver.java
* Removed: Android native camera modules
* Updated: DeviceScreen.h to display properties with page orientation
* Updated: IInputRecognier to include a Finish event (when no more input leads to finish of the gesture)
* Fixed: Bug in iosx calculating the tick count (Environs.utils.cpp)
* Updated: Android platform layer to secure access to portal generators/receivers map
* Fixed: Bug in start of AsyncWorker thread
* Fixed: Bug in loading of gesture recognizer modules (Android platform layer)
* Added: options streamBitrateKB and portalAutoAccept (only for storage)
* Addedd: iOS HW h264 encoder
* Updated: Android portal color format detection
* Updated: SendTcpPortal to support a prefix for SPS/PPS
* Enabled: ignoreBufferStatus for Android portal receiver
* Fixed: Bug in DecoderBase that skipped calculation of stride
* Updated: Android platform layer portal generator now using correct color formats
* Fixed: Bug in HandleOptionsMessage (NOTIFY_PORTAL_LOCATION_CHANGED) of DeviceBase which copied the memory of the address of an address
* Updated: Android platform layer and native layer to allow platform portal generators
* Excluded: Android jni modules for camera access and hw encoding/decoding. We rely on the platform layer implementation.
* Updated: Android platform camera/encoder for portals (using a texturesurface API 18, 4.3)

* Changed: Replaced IPortalInterface references in Interfaces with the (collection) workerStages
* Added: Platform specific portal generator classes for (Android / iOS)


### v 0.6.0.5551 (Release)

---------------------------

* Reduced: Amount of log messages
* Fixed: Bug in LocateLoadModule (export.cpp)
* Changed: Library Method setUseNativeDecoder to setUseNativeDecoder1 (handling of third party decoder)
* Changed: Android use aar archive
* Added: Android Class LibLicThread to download and present license of 3rd party libraries
* Added: load3rd to Libs (Android) for downloading third party libraries
* Fixed: Bug in LocateLoadModule (due to change of modulename without extension)
* Reduced: Amount of log messages
* Fixed: Bug in j2c
* Removed: Some obsolete compiler flags
* Fixed: Bug in HandleStringMessage of DeviceBase
* Changed: Message protocol ids
* Added: O1 access to TcpListener, BulkListener, and PortalHandling
* Added: String array creation to j2c
* Changed: Type qualifier in messge protocol
* Updated: Android project files and download helper
* Updated: Added option 3/4 to download scripts to rescue/restore downloaded files
* Updated: Download helper and prepare helper to reflect the new folder structure
* Changed: Restructured Android Studio project directories
* Updated: GetCallback of IPortalDecoder to also query callbackType
* Fixed: Usage of DECODER_CALLBACL_TYPE in Decoder.Android.h
* Updated: Types for render callback RENDER_CALLBACK_TYPE_* used in setRenderCallback and as type for the callback parameter
* Added: GDI decoder for images to Windows Decoder
* Renewed repository for 0.6.0 release




### v 0.5.0.5538

---------------------------

* Latest release from svn repository



### v 0.5.0 (rev 5417/5418)

---------------------------

* Initial import to master branch (from svn repository)

