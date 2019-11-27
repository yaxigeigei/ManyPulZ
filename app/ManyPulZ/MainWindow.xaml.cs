using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;

namespace ManyPulZ
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        static MainWindow MWInstance;
        static SerialPort serialPort = new SerialPort();
        Thread serialReadThread;
        static bool reading = false;
        static bool thisDevice = false;

        SaveAndLoad saveLoad = new SaveAndLoad();
        Parameters paras = new Parameters();


        public MainWindow()
        {
            InitializeComponent();

            MWInstance = this;

            serialPort.BaudRate = 115200;
            serialPort.ReadTimeout = 500;
            serialPort.WriteTimeout = 500;

            string[] portsAvailable = SerialPort.GetPortNames();
            foreach (string port in portsAvailable)
            {
                comComboBox.Items.Add(port);
                comComboBox.SelectedItem = port;
                try
                {
                    serialPort.Open();

                    serialReadThread = new Thread(new ThreadStart(SerialRead));
                    serialReadThread.IsBackground = true;
                    reading = true;
                    serialReadThread.Start();

                    SerialSend("MPZ");
                    Thread.Sleep(50);
                    if (thisDevice)
                    {
                        oneTimeCfg();
                        fireButton.IsEnabled = true;
                        connectButton.IsEnabled = false;
                    }
                    else
                    {
                        reading = false;
                        serialReadThread.Join();
                        serialPort.Close();
                    }
                }
                catch (Exception)
                {
                    comComboBox.SelectedIndex = -1;
                }
            }

            if (File.Exists(saveLoad.settingsFilePath))
                paras = (Parameters)saveLoad.LoadAll();
            else
                saveLoad.SaveAll(paras);

            parasGrid.DataContext = paras;


        }


        private static void SerialRead()
        {
            while (reading)
            {
                try
                {
                    string feedback = serialPort.ReadLine();
                    MainWindow.MWInstance.serialMonitorListBox.Dispatcher.BeginInvoke((ThreadStart)delegate
                    {
                        ListBoxItem lbi = new ListBoxItem();
                        lbi.Content = feedback.Substring(0, feedback.Length - 1);
                        lbi.Foreground = Brushes.SeaGreen;
                        MainWindow.MWInstance.serialMonitorListBox.Items.Add(lbi);
                        MainWindow.MWInstance.serialMonitorListBox.ScrollIntoView(lbi);
                    });
                    if (!String.IsNullOrEmpty(feedback))
                    {
                        if (feedback == "YES\r")
                            thisDevice = true;
                    }
                }
                catch (TimeoutException) { }
                Thread.Sleep(5);
            }
        }
        private bool SerialSend(string cmd)
        {
            bool success = false;
            try
            {
                serialPort.Write(cmd);
                success = true;

                ListBoxItem lbi = new ListBoxItem();
                lbi.Content = cmd;
                serialMonitorListBox.Items.Add(lbi);
                serialMonitorListBox.ScrollIntoView(lbi);
            }
            catch (InvalidOperationException)
            {
                MessageBox.Show("Connection Lost", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                serialPort.Close();
                connectButton.IsEnabled = true;
                fireButton.IsEnabled = false;
            }
            return success;
        }




        private void oneTimeCfg()
        {
            
        }

        private void serialMonitorListBox_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            serialMonitorListBox.Items.Clear();
        }

        private void Window_Closing_1(object sender, System.ComponentModel.CancelEventArgs e)
        {
            saveLoad.SaveAll(paras);
        }

        private void comComboBox_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (comComboBox.SelectedIndex != -1)
                serialPort.PortName = (string)comComboBox.SelectedItem;
        }

        private void connectButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                serialPort.Open();

                serialReadThread = new Thread(new ThreadStart(SerialRead));
                serialReadThread.IsBackground = true;
                reading = true;
                serialReadThread.Start();

                SerialSend("MPZ");
                Thread.Sleep(50);
                if (thisDevice)
                {
                    oneTimeCfg();
                    fireButton.IsEnabled = true;
                    connectButton.IsEnabled = false;
                }
                else
                {
                    reading = false;
                    serialReadThread.Join();
                    serialPort.Close();
                    MessageBox.Show("Incorrect Device", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
            catch (Exception)
            {
                MessageBox.Show("Connection Failed", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void fireButton_Click(object sender, RoutedEventArgs e)
        {
            SerialSend(paras.pulseWidth + "w" + paras.pulseNum + "m" + paras.pulseInterval + "i" + paras.repeatNum + "r" + paras.repeatInterval + "v" + "p");
        }

        private void minButton_Click(object sender, RoutedEventArgs e)
        {
            this.WindowState = System.Windows.WindowState.Minimized;
        }

        private void closeButton_Click(object sender, RoutedEventArgs e)
        {
            reading = false;

            try
            {
                serialReadThread.Join();
                serialPort.Close();
            }
            catch (Exception) { }

            Application.Current.Shutdown();
        }

        // Drag to move the window
        private void Window_MouseLeftButtonDown_1(object sender, MouseButtonEventArgs e)
        {
            try { this.DragMove(); }
            catch (Exception) { }                                       // Strange error occurs when not catching 
        }


    }




    [Serializable]
    class Parameters
    {
        public int pulseWidth { get; set; }
        public int pulseInterval { get; set; }
        public int pulseNum { get; set; }
        public int repeatNum { get; set; }
        public int repeatInterval { get; set; }

        public Parameters()
        {
            pulseWidth = 10;
            pulseInterval = 10;
            pulseNum = 10;
            repeatNum = 5;
            repeatInterval = 3000;
        }

    }




    class SaveAndLoad
    {
        public string settingsFilePath;

        public SaveAndLoad()
        {
            // Create a folder in My Document for saving settings
            string settingsFolderPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) +
                Path.DirectorySeparatorChar + "ManyPulZ";
            if (!Directory.Exists(settingsFolderPath)) { Directory.CreateDirectory(settingsFolderPath); }

            // Get the path of settings file
            settingsFilePath = settingsFolderPath + Path.DirectorySeparatorChar + "Settings.bin";
        }

        public void SaveAll(Object alset)
        {
            BFormatter.save(settingsFilePath, alset);
        }

        public object LoadAll()
        {
            return BFormatter.load(settingsFilePath);
        }
    }

    class BFormatter
    {
        /// <summary>
        /// BinaryFormatter序列化方式
        /// </summary>
        /// <param name="filepath"></param>
        /// <param name="obj"></param>
        public static void save(string filepath, object obj)
        {
            IFormatter formatter = new BinaryFormatter();
            Stream fs = null;
            try
            {
                fs = new FileStream(filepath, FileMode.Create, FileAccess.Write, FileShare.Write);
                formatter.Serialize(fs, obj);
            }
            catch (Exception e) { throw e; }
            finally
            { if (fs != null) { fs.Close(); } }
        }


        public static object load(string filepath)
        {
            IFormatter formatter = new BinaryFormatter();
            Stream fs = null;
            try
            {
                fs = new FileStream(filepath, FileMode.Open, FileAccess.Read, FileShare.Read);
                return formatter.Deserialize(fs);
            }
            catch (Exception e)
            {
                throw e;
            }
            finally { if (fs != null) { fs.Close(); } }
        }
    }

}
