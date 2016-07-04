using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;


namespace environs
{
    /// <summary>
    /// Interaction logic for DeviceItem.xaml
    /// </summary>
    public partial class OscilloscopeSimple : UserControl
    {
        private const string className = "OscilloscopeSimple";

        String title = "Unknown";

        int[] values = null;

        int TITLE_SIZE = 12;
        int xMax;
        int yRangeUpper;
        int xCurrent;
        bool curUpdated;

        int yMax;
        int yMidline;
        int yRangeHalfInPixel;
        int yOffset;

        float halfYRange;

        float yRangeMax;
        float curValue;

        SolidColorBrush brushBefore = new SolidColorBrush(Colors.GreenYellow);
        SolidColorBrush brushAfter = new SolidColorBrush(Colors.Green);
        int margin = 2;

        Polyline graphBefore = new Polyline();
        Polyline graphAfter = new Polyline();

        Line lineMiddle = new Line();

        TextBlock textTitle = new TextBlock();
        TextBlock textMin = new TextBlock();
        TextBlock textMax = new TextBlock();
        TextBlock textCurrentValue = new TextBlock();

        public OscilloscopeSimple()
        {
            InitializeComponent();

            this.SizeChanged += OscilloscopeSimple_SizeChanged;
            CalculateOsziDimensions();

            for (int i = 0; i < xMax; i++)
                values[i] = yMidline + 5;

            yRangeMax = 1.0f;
            
            xCurrent = 0;
            curUpdated = false;

            osciCanvas.Children.Add(lineMiddle);

            textTitle.Text = title;
            textTitle.Foreground = new SolidColorBrush(Colors.Yellow);
            osciCanvas.Children.Add(textTitle);
            
            textMin.Text = yRangeMax.ToString("-0.00");
            textMin.Foreground = new SolidColorBrush(Colors.Yellow);
            osciCanvas.Children.Add(textMin);

            textMax.Text = yRangeMax.ToString("0.00");
            textMax.Foreground = new SolidColorBrush(Colors.Yellow);
            osciCanvas.Children.Add(textMax);

            textCurrentValue.Text = "-15.00";
            textCurrentValue.Foreground = new SolidColorBrush(Colors.Yellow);
            osciCanvas.Children.Add(textCurrentValue);

            graphBefore.Stroke = brushBefore;
            graphBefore.StrokeThickness = 2;
            osciCanvas.Children.Add(graphBefore);

            graphAfter.Stroke = brushAfter;
            graphAfter.StrokeThickness = 2;
            osciCanvas.Children.Add(graphAfter);
        }


        void OscilloscopeSimple_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            CalculateOsziDimensions();
        }


        private void CalculateOsziDimensions()
        {
            lock (this)
            {
                yMax = (int)this.ActualHeight;

                yRangeUpper = yMax - TITLE_SIZE;

                xMax = (int)this.ActualWidth;

                yMidline = (int)this.ActualHeight / 2;
                yRangeHalfInPixel = yMidline - TITLE_SIZE;

                yOffset = TITLE_SIZE;

                halfYRange = yMidline - TITLE_SIZE;

                if (xMax > 0)
                {
                    xMax -= margin*2;

                    values = new int[xMax];
                    for (int i = 0; i < xMax; i++)
                        values[i] = yMidline - 5;

                    lineMiddle.X1 = 0;
                    lineMiddle.Y1 = yMidline;
                    lineMiddle.X2 = xMax + (margin * 2) - 2;
                    lineMiddle.Y2 = yMidline;
                    lineMiddle.StrokeThickness = 2f;
                    lineMiddle.Stroke = new SolidColorBrush(Colors.Blue);
                    lineMiddle.Opacity = 1.0f;

                    Canvas.SetLeft(textTitle, xMax/2 - textTitle.ActualWidth);
                    Canvas.SetTop(textTitle, yMax - textTitle.ActualHeight- 4);
                    
                    Canvas.SetLeft(textMin, 1);
                    Canvas.SetTop(textMin, yMax - textTitle.ActualHeight - 4);

                    Canvas.SetLeft(textMax, 1);
                    Canvas.SetTop(textMax, 0);

                    Canvas.SetLeft(textCurrentValue, xMax - textCurrentValue.ActualWidth);
                    Canvas.SetTop(textCurrentValue, 0);
                }

            }
        }


        public void SetTitle(String _title)
        {
            title = _title;

            EnvironsAddon.Dispatch(new Action(delegate()
            {
                textTitle.Text = title;
            }));
        }


        public void UpdateValue(double value1)
        {
            value1 = -value1;

            float value = (float) value1;
            lock (this)
            {
                float yValue = Math.Abs(value);
                if (yValue > yRangeMax)
                {
                    // Recalculate graphBefore values
                    float yRangeMaxNew = yValue * 1.6f;

                    for (int i = 0; i < xMax; i++)
                    {
                        int o = values[i];

                        int v;
                        if (o >= yMidline)
                            v = o - yMidline;
                        else
                            v = yMidline - o;

                        float rebase = ((float)v * yRangeMax) / yRangeHalfInPixel;

                        float newValue = (rebase * yRangeHalfInPixel) / yRangeMaxNew;
                        if (o >= yMidline)
                            values[i] = yMidline + (int)newValue;
                        else
                            values[i] = yMidline - (int)newValue;
                    }
                    yRangeMax = yRangeMaxNew;

                    EnvironsAddon.Dispatch(new Action(delegate()
                    {
                        textMin.Text = yRangeMax.ToString("-0.00");
                        textMax.Text = yRangeMax.ToString("0.00");
                    }));
                }

                yValue = (yValue * yRangeHalfInPixel) / yRangeMax;
                if (value < 0)
                    values[xCurrent] = yMidline - (int)yValue;
                else
                    values[xCurrent] = yMidline + (int)yValue;

                curValue = value;
                curUpdated = true;
            }
        }


        public void IncreaseTimer()
        {
            if (values == null)
                return;

            lock (this)
            {
                if (!curUpdated)
                {
                    if (xCurrent == 0)
                    {
                        values[xCurrent] = values[xMax - 1];
                    }
                    else
                    {
                        values[xCurrent] = values[xCurrent - 1];
                    }
                }

                xCurrent++;
                if (xCurrent >= xMax)
                    xCurrent = 0;

                curUpdated = false;
            }

            EnvironsAddon.Dispatch(new Action(delegate()
            {
                Draw();
            }));
        }


        PointCollection pointsBefore = new PointCollection();
        PointCollection pointsAfter = new PointCollection();

        private void Draw()
        {
            if (values == null)
                return;

            pointsBefore = new PointCollection();

            for (int i = 0; i < xCurrent; i++)
            {
                pointsBefore.Add(new Point(i + margin, values[i]));
            }

            graphBefore.Points = pointsBefore;

            pointsAfter = new PointCollection();

            if (xCurrent + 1 < xMax)
            {
                for (int i = xCurrent + 1; i < xMax; i++)
                {
                    pointsAfter.Add(new Point(i + margin, values[i]));
                }
            }

            graphAfter.Points = pointsAfter;

            textCurrentValue.Text = (-curValue).ToString("0.00");
        }

        /*
        public static OszilloscopeSimple DeviceAppeared(DeviceInstance deviceInstance, ItemCollection itemCollection)
        {
            string key = buildKey(deviceInstance.deviceID_, deviceInstance.areaName_, deviceInstance.appName_);

            if (deviceItems.ContainsKey(key))
                return null;

            DeviceItem device = new DeviceItem();

            device.deviceInstance = deviceInstance;

            device.items = itemCollection;

            device.scatterItem = new ScatterViewItem();
            device.scatterItem.Content = device;
            device.scatterItem.Opacity = 1f;
            device.scatterItem.IsEnabled = true;
            device.scatterItem.CanScale = true;
            device.scatterItem.CanMove = true;
            
            //device.scatterItem.Background = Brushes.Transparent;

            SolidColorBrush br = new SolidColorBrush(Color.FromArgb(0xE6, 0xBF, 0xBF, 0xBF));
            br.Opacity = 0.6f;
            device.scatterItem.Background = br;

            device.textDesc.Foreground = forgroundBrush;
            device.borderMain.BorderBrush = forgroundBrush;
            device.borderDesc.BorderBrush = forgroundBrush;

            device.IdentifyAs(key);
            return device;
        }
        */

        void deviceItem_SizeChanged(object sender, SizeChangedEventArgs e)
        {          
        }

    }
}
