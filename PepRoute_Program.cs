using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PepRoute
{
    class Program
    {
        static void Main(string[] args)
        {
            MyLine line1 = new MyLine { name = "Line1", startPoint = new MyPoint { name = "Point1", lat = 13.26084171F, lng = 74.89187972F }, endPoint = new MyPoint { name = "Point2", lat = 13.24221111F, lng = 74.9438502F } };
            MyLine line2 = new MyLine { name = "Line2", startPoint = new MyPoint { name = "Point2", lat = 13.23990372F, lng = 74.95484591F }, endPoint = new MyPoint { name = "Point3", lat = 13.22519861F, lng = 74.97930765F } };
            MyLine line3 = new MyLine { name = "Line3", startPoint = new MyPoint { name = "Point3", lat = 13.2252084F, lng = 74.97916953F }, endPoint = new MyPoint { name = "Point4", lat = 13.23907802F, lng = 74.9832894F } };

            List<MyLine> routeLines = new List<MyLine> { line1, line2, line3 };

            MyPoint routePoint = new MyPoint { lat = 13.2338881F, lng = 74.97422218F };

            MyPoint closestPoint = null;

            MyLine closestLine = GetClosestLine(routeLines, routePoint, out closestPoint);
        }

        private static MyLine GetClosestLine(List<MyLine> lines, MyPoint point, out MyPoint closestPoint)
        {
            double shortDistance = -1;
            MyLine closestLine = null;
            closestPoint = null;

            double[] pointC = { point.lng, point.lat };

            foreach (MyLine line in lines)
            {
                
                //double shortestDistance = GetPointDistance(closePoint, point);

                double[] pointA = {line.startPoint.lng, line.startPoint.lat};
                double[] pointB = { line.endPoint.lng, line.endPoint.lat };
                double shortestDistance = LineToPointDistance2D(pointA, pointB, pointC, true);

                if (shortDistance < 0 || shortestDistance < shortDistance)
                {
                    shortDistance = shortestDistance;
                    closestLine = line;
                    //closestPoint = closePoint;
                }
            }
            return closestLine;
        }

        //Compute the dot product AB . AC
        private static double DotProduct(double[] pointA, double[] pointB, double[] pointC)
        {
            double[] AB = new double[2];
            double[] BC = new double[2];
            AB[0] = pointB[0] - pointA[0];
            AB[1] = pointB[1] - pointA[1];
            BC[0] = pointC[0] - pointB[0];
            BC[1] = pointC[1] - pointB[1];
            double dot = AB[0] * BC[0] + AB[1] * BC[1];

            return dot;
        }

        //Compute the cross product AB x AC
        private static double CrossProduct(double[] pointA, double[] pointB, double[] pointC)
        {
            double[] AB = new double[2];
            double[] AC = new double[2];
            AB[0] = pointB[0] - pointA[0];
            AB[1] = pointB[1] - pointA[1];
            AC[0] = pointC[0] - pointA[0];
            AC[1] = pointC[1] - pointA[1];
            double cross = AB[0] * AC[1] - AB[1] * AC[0];

            return cross;
        }

        //Compute the distance from A to B
        private static double Distance(double[] pointA, double[] pointB)
        {
            double d1 = pointA[0] - pointB[0];
            double d2 = pointA[1] - pointB[1];

            return Math.Sqrt(d1 * d1 + d2 * d2);
        }

        //Compute the distance from AB to C
        //if isSegment is true, AB is a segment, not a line.
        private static double LineToPointDistance2D(double[] pointA, double[] pointB, double[] pointC, bool isSegment)
        {
            double dist = CrossProduct(pointA, pointB, pointC) / Distance(pointA, pointB);
            if (isSegment)
            {
                double dot1 = DotProduct(pointA, pointB, pointC);
                if (dot1 > 0)
                    return Distance(pointB, pointC);

                double dot2 = DotProduct(pointB, pointA, pointC);
                if (dot2 > 0)
                    return Distance(pointA, pointC);
            }
            return Math.Abs(dist);
        } 

        //======================================================================================================

        private static MyPoint GetClosestPoint(MyLine line, MyPoint point)
        {
            double[] a_to_p = { point.lng - line.startPoint.lng, point.lat - line.startPoint.lat };                 //Storing vector A->P
            double[] a_to_b = { line.endPoint.lng - line.startPoint.lng, line.endPoint.lat - line.startPoint.lat }; //Storing vector A->B

            double atb2 = Math.Pow(a_to_b[0], 2) + Math.Pow(a_to_b[1], 2);                                          // Basically finding the squared magnitude of a_to_b

            double atp_dot_atb = a_to_p[0] * a_to_b[0] + a_to_p[1] * a_to_b[1];                                     //The dot product of a_to_p and a_to_b

            double t = atp_dot_atb / atb2;                                                                          //The normalized "distance" from a to your closest point

            MyPoint myPoint = new MyPoint { lat = line.startPoint.lat + a_to_b[1] * t, lng = line.startPoint.lng + a_to_b[0] * t }; //Add the distance to A, moving towards B

            return myPoint;
        }

        private static double GetPointDistance(MyPoint startPoint, MyPoint endPoint)
        {
            return Math.Sqrt((Math.Pow(endPoint.lng - startPoint.lng, 2) + Math.Pow(endPoint.lat - startPoint.lat, 2)));
        }
    }



    public class MyLine
    {
        public string name { get; set; }
        public MyPoint startPoint { get; set; }
        public MyPoint endPoint { get; set; }
    }

    public class MyPoint
    {
        public string name { get; set; }
        public double lat { get; set; }
        public double lng { get; set; }
    }
}
