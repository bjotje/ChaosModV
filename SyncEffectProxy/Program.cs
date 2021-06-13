using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SyncEffectProxy
{
    class Program
    {
        static void Main(string[] args)
        {
            ChaosPipe.ChaosPipeClient cpc = new ChaosPipe.ChaosPipeClient();

            while (cpc.IsConnected()) { }
        }
    }
}
