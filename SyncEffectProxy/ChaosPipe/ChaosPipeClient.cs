using Microsoft.AspNet.SignalR.Client;
using Serilog;
using System;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Threading.Tasks;
using System.Timers;

namespace SyncEffectProxy.ChaosPipe
{
    class ChaosPipeClient
    {
        /// <summary>
        /// Speed at which the chaos mod pipe gets processed
        /// </summary>
        public static readonly int PIPE_TICKRATE = 100;

        private ILogger logger = Log.Logger.ForContext<ChaosPipeClient>();
        private NamedPipeClientStream pipe = new NamedPipeClientStream(
            ".",
            "ChaosModVMultiplayerClientPipe",
            PipeDirection.InOut,
            PipeOptions.Asynchronous);
        private StreamReader pipeReader;
        private Timer pipeTick = new Timer();
        private StreamWriter pipeWriter;
        private Task<string> readPipeTask;
        public ChaosPipeClient()
        {
            ConnectAsync();
            // Setup pipe tick
            pipeTick.Interval = PIPE_TICKRATE;
            pipeTick.Elapsed += PipeTick;

            // Connect to the chaos mod pipe
            try
            {
                pipe.Connect(1000);
                pipeReader = new StreamReader(pipe);
                pipeWriter = new StreamWriter(pipe);
                pipeWriter.AutoFlush = true;


                SendMessageToPipe("hello");

                logger.Information("successfully connected to chaos mod pipe");

                pipeTick.Enabled = true;
            }
            catch (Exception e)
            {
                logger.Fatal(e, "failed to connect to chaos mod pipe, aborting");
                return;
            }
        }




        /// <summary>
        /// Check if the chaos mod pipe is still connected
        /// </summary>
        /// <returns>If the chaos mod pipe is still connected</returns>
        public bool IsConnected()
        {
            return pipe.IsConnected;
        }

        /// <summary>
        /// Disconnects the stream reader/writer and the pipe itself
        /// </summary>
        private void DisconnectFromPipe()
        {
            pipeReader.Close();
            pipeWriter.Close();
            pipe.Close();
        }

        /// <summary>
        /// Gets called every pipe tick
        /// </summary>
        private void PipeTick(object sender, ElapsedEventArgs e)
        {
            try
            {
                logger.Information("PipeTick!");
                SendHeartBeat();
                ReadPipe();
            }
            catch (Exception exception)
            {
                logger.Fatal(exception, "chaos mod pipe tick failed, disconnecting");
                DisconnectFromPipe();
            }
        }
        /// <summary>
        /// Reads the contents of the chaos mod pipe and evaluates its message
        /// </summary>
        private void ReadPipe()
        {
            // If no reading task is active, create one
            if (readPipeTask == null) readPipeTask = pipeReader.ReadLineAsync();
            // If the reading task is created and complete, get its results
            else if (readPipeTask.IsCompleted)
            {
                // Get the message from the pipe read
                var message = readPipeTask.Result;
                // Null the reading task so the next read is dispatched
                readPipeTask = null;


                if (message.StartsWith("broadcasteffect;"))
                {
                    string[] splittedmessage = message.Split(';');
                    string effectid = splittedmessage[1];

                    hubProxy.Invoke("broadcasteffect", effectid);
                }
                else
                {
                    logger.Warning($"unknown request: {message}");
                }

            }
        }
        /// <summary>
        /// Sends a message to the chaos mod pipe
        /// </summary>
        /// <param name="message">Message to be sent</param>
        private void SendMessageToPipe(string message)
        {
            try
            {
                pipeWriter.Write($"{message}\0");
            }
            catch (Exception e)
            {
                logger.Information(e, "error that ocurred when writing pipe");
                DisconnectFromPipe();
            }
        }

        ///// <summary>
        ///// Sends a heartbeat to the chaos mod
        ///// </summary>
        private void SendHeartBeat()
        {
            SendMessageToPipe("ping");
        }

        //Using signalr because it was the easiest for me to get going. Replace?
        private IHubProxy hubProxy { get; set; }
        const string ServerURI = "http://localhost:8087/signalr";
        private HubConnection Connection { get; set; }

        private async void ConnectAsync()
        {
            Connection = new HubConnection(ServerURI);
            Connection.Closed += Connection_Closed;
            Connection.Error += Connection_Error;
            Connection.StateChanged += Connection_StateChanged;
            hubProxy = Connection.CreateHubProxy("MyHub");

            hubProxy.On<int>("DispatchEffect", (enumID) =>
                SendMessageToPipe("dispatcheffect;" + enumID.ToString())
            );

            try
            {
                await Connection.Start();
            }
            catch (Exception ex)
            {

            }

        }

        private void Connection_StateChanged(StateChange obj)
        {
        }

        private void Connection_Error(Exception obj)
        {
            SendMessageToPipe("signalrclosed");
        }

        private void Connection_Closed()
        {
            SendMessageToPipe("signalrclosed");
        }
    }
}
